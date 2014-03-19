/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/serial_reg.h>
#include <linux/circ_buf.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>

#include "rk_ipc_pipe.h"
#include "rk_ipc.h"


#define PORT_RK_IPC     91

#define DRV_NAME				"rk_ipc-serial"

#define IPC_SERIAL_PORT_NUM_MAX		(16)

#define IPC_SERIAL_MINOR		(230)

#define	IPCSERIAL_FIFO_SIZE		(1024)


struct ipc_serial_port {
	struct uart_port	u_port;

	struct device *dev;

	struct ipc_pipe *pipe;
	struct work_struct work;

	struct notifier_block recv_nb;

	int send_pending;
};

static struct workqueue_struct *ipc_serial_workqueue;

static struct uart_driver rk_ipc_uart = {
	.owner			= THIS_MODULE,
	.driver_name	= "ipc_serial",
	.dev_name		= "ttyP",
	.major			= TTY_MAJOR,
	.minor			= IPC_SERIAL_MINOR,
	.nr				= IPC_SERIAL_PORT_NUM_MAX,
	.cons			= NULL,
};


/*------------------------------------------------------------------------*/

static void ipc_serial_send_data(struct ipc_serial_port *isp);

static void ipc_serial_work(struct work_struct *work)
{
	struct ipc_serial_port *isp = container_of(work, struct ipc_serial_port, work); 
	struct uart_port *port;
	struct tty_struct *tty;
	struct tty_port *ttyport;
	unsigned long flags;

	port = &(isp->u_port);

	/* Are there any data left for ipc pipe resource limit?
	 * if true, try to continue send data
	 */
	if (isp->send_pending) {
		isp->send_pending = 0;

		spin_lock_irqsave(&port->lock, flags);
		ipc_serial_send_data(isp);
		spin_unlock_irqrestore(&port->lock, flags);
	}

	/* handle incoming data */
	ttyport = &(port->state->port);
	tty = tty_port_tty_get(ttyport);

	if (NULL != tty) {
		size_t read_avail;
		size_t avail_tty;
		size_t read_actual;
		unsigned char *ptr;

		do{
			read_avail = ipc_pipe_read_avail(isp->pipe);
			if (read_avail <= 0){
			 	break;
			}
			else{
				do{
				avail_tty = tty_prepare_flip_string(tty, &ptr, read_avail);
				}while(avail_tty==0);
				if(avail_tty > 0){
					read_actual=ipc_pipe_read(isp->pipe, ptr, avail_tty);
					if (read_actual != avail_tty && read_actual != -1) {
						//shouldn't be happened.
						dev_err(isp->dev, "OOPS - ipc pipe avail bytes mismatch?! read_actual=%d",read_actual);
					}
					tty_flip_buffer_push(tty);
				}
			}
		}while(1);
		
		tty_kref_put(tty);
	} else {
		dev_err(isp->dev, "tty_struct is NULL \n");
	}

}

static void ipc_serial_send_data(struct ipc_serial_port *isp)
{
	struct uart_port *port;
	int cnt_to_end, count, len;
	struct circ_buf *xmit;

	port = &isp->u_port;
	xmit = &port->state->xmit;
	//IPC_DBG(DEBUG_INFO,"=============%s: pipe=%d\n",__func__,isp->u_port.line);
	if (port->x_char) {
		ipc_pipe_write(isp->pipe, &port->x_char, 1);
		port->icount.tx++;
		port->x_char = 0;
		return;
	}

	len = uart_circ_chars_pending(xmit);
	if (len <= 0) {
		return;
	}

	do {
		cnt_to_end = CIRC_CNT_TO_END(xmit->head, xmit->tail, UART_XMIT_SIZE);

		count = ipc_pipe_write(isp->pipe, xmit->buf + xmit->tail, cnt_to_end);

		BUG_ON(count < 0);

		/* data not to be sent fully for ipc pipe resouce limit,
		 * so here set a flag, when more ipc pipe resource available,
		 * send continue.
		 */
		if (count == 0) {
			//IPC_DBG(DEBUG_INFO,"%s count == 0 cnt_to_end=%d\n",__func__,cnt_to_end);
			isp->send_pending = 1;
			break;
		}

		xmit->tail = (xmit->tail + count) & (UART_XMIT_SIZE -1);
		port->icount.tx += count;

		len = uart_circ_chars_pending(xmit);

	} while ( len > 0);

	if (len < WAKEUP_CHARS)
		uart_write_wakeup(port);
}

static void ipc_serial_start_tx(struct uart_port *port)
{
	struct ipc_serial_port *isp = (struct ipc_serial_port*)port;
	ipc_serial_send_data(isp);
}

static void ipc_serial_stop_tx(struct uart_port *port)
{
}

static void ipc_serial_stop_rx(struct uart_port *port)
{
}

/* Return TIOCSER_TEMT when transmitter FIFO is empty, otherwise return 0.*/
static unsigned int ipc_serial_tx_empty(struct uart_port *port)
{
	return TIOCSER_TEMT;
}


static int ipc_pipe_notifier(struct notifier_block *nb, unsigned long action, void *data)
{
	struct ipc_serial_port *isp = container_of(nb, struct ipc_serial_port, recv_nb);
	switch (action) {
	case RK_IPCPIPE_EVENT_NEW_DATA:
	case RK_IPCPIPE_EVENT_MORE_SPACE:
		queue_work(ipc_serial_workqueue, &isp->work);
		break;
	default:
		break;
	}

	return 0;
}



static int ipc_serial_startup(struct uart_port *port)
{
	struct ipc_serial_port *isp = (struct ipc_serial_port*)port;
	struct ipc_pipe* pipe;	
	pipe = ipc_pipe_open(isp->u_port.line);
	if (pipe == NULL){
		IPC_DBG(DEBUG_ERR,"%s:ipc_pipe_open err %d\n",__func__,isp->u_port.line);
		return -ENODEV;
	}
	
	INIT_WORK(&isp->work, ipc_serial_work);	
	IPC_DBG(DEBUG_INFO,"%s ttyP%d\n",__func__,isp->u_port.line);

	ipc_pipe_config(pipe, RK_IPC_PIPE_CFG_SNDBUF_QUANUM_SET, 2048);//256 	
	ipc_pipe_config(pipe, RK_IPC_PIPE_CFG_SNDBUF_QSET_SET, 32); 	
	ipc_pipe_config(pipe, RK_IPC_PIPE_CFG_SNDBUF_PREALLOC, 0); 	
	isp->pipe = pipe;

	isp->recv_nb.notifier_call	= ipc_pipe_notifier;
	ipc_pipe_notifier_register(pipe, &isp->recv_nb);

	return 0;
}

static void ipc_serial_shutdown(struct uart_port *port)
{
	struct ipc_serial_port *isp = (struct ipc_serial_port*)port;
	IPC_DBG(DEBUG_INFO,"%s ttyP%d\n",__func__,isp->u_port.line); 
	if (isp->pipe) {	
		ipc_pipe_close(isp->pipe);
		isp->pipe = NULL;
	}

}

static void ipc_serial_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old)
{
}

static void ipc_serial_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
}

static unsigned int ipc_serial_get_mctrl(struct uart_port *port)
{
	return 0;
}

static void ipc_serial_enable_ms(struct uart_port *port)
{
}

static void ipc_serial_release_port(struct uart_port *port)
{
}

static int ipc_serial_request_port(struct uart_port *port)
{
	return 0;
}

static void ipc_serial_config_port(struct uart_port *port, int flags)
{
}

struct uart_ops ipc_serial_uart_ops = {
	.startup	= ipc_serial_startup,
	.shutdown	= ipc_serial_shutdown,
	.tx_empty	= ipc_serial_tx_empty,
	.stop_tx	= ipc_serial_stop_tx,
	.start_tx	= ipc_serial_start_tx,
	.stop_rx	= ipc_serial_stop_rx,
	.set_termios	= ipc_serial_set_termios,
	.set_mctrl	= ipc_serial_set_mctrl,
	.get_mctrl	= ipc_serial_get_mctrl,
	.enable_ms	= ipc_serial_enable_ms,
	.type		= NULL,
	.release_port	= ipc_serial_release_port,
	.request_port	= ipc_serial_request_port,
	.config_port	= ipc_serial_config_port,
};


static int ipc_serial_probe(struct platform_device *pdev)
{
	struct ipc_serial_port *isp;
	int line;
	int ret;

	line = (pdev->id >= 0) ? pdev->id : 0;
	if (line >= IPC_SERIAL_PORT_NUM_MAX)
		return -ENXIO;

	pr_info("IPC serial: detected port #%d\n", line);

	isp = kzalloc(sizeof(struct ipc_serial_port), GFP_KERNEL);
	if (unlikely(isp == NULL)) {
		dev_err(&pdev->dev, "Unable to malloc ipc_serial_port\n");
		return -ENOMEM;
	}

	isp->dev = &pdev->dev;
	isp->u_port.dev = &pdev->dev;
	isp->u_port.type = PORT_RK_IPC;
	isp->u_port.iotype = UPIO_MEM;
	isp->u_port.fifosize = IPCSERIAL_FIFO_SIZE;
	isp->u_port.ops = &ipc_serial_uart_ops;
	isp->u_port.line = line;

	platform_set_drvdata(pdev, isp);

	ret = uart_add_one_port(&rk_ipc_uart, &(isp->u_port));
	if (ret != 0) {
		dev_err(&pdev->dev, "Unable to add one uart port\n");
		platform_set_drvdata(pdev, NULL);
		kfree(isp);
		return ret;
	}

	return 0;
}

static int __devexit  ipc_serial_remove(struct platform_device *pdev)
{
	struct ipc_serial_port *isp = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	if (unlikely(NULL != isp)) {
		uart_remove_one_port(&rk_ipc_uart, &(isp->u_port));
		kfree(isp);
	}

	return 0;
}


static struct platform_driver ipc_serial_driver = {
	.probe      = ipc_serial_probe,
	.remove		= __devexit_p(ipc_serial_remove),
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init ipc_serial_init(void)
{
	int ret;

	pr_info("IPC Serial Driver, (c) 2013 Rock-chips Inc.\n");

	ipc_serial_workqueue = create_workqueue("rk_ipc_serial");
	if (unlikely(!ipc_serial_workqueue)) {
		return -ENOMEM;
	}

	ret = uart_register_driver(&rk_ipc_uart);
	if (ret != 0) {
		return ret;
	}

	ret = platform_driver_register(&ipc_serial_driver);
	if (ret != 0) {
		uart_unregister_driver(&rk_ipc_uart);
		return ret;
	}


	return ret;
}


static void __exit ipc_serial_exit(void)
{
	platform_driver_unregister(&ipc_serial_driver);
	uart_unregister_driver(&rk_ipc_uart);
	destroy_workqueue(ipc_serial_workqueue);
}

module_init(ipc_serial_init);
module_exit(ipc_serial_exit);

MODULE_AUTHOR("rock-chips");
MODULE_DESCRIPTION("rock-chips IPC serial driver");
MODULE_LICENSE("GPL");

