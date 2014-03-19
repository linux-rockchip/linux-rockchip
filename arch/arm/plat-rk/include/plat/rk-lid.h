#ifndef __RK_LID_H__
#define __RK_LID_H__

struct rk_lid_platform_data {
	int debounce_interval ;//debounce interval ,ms
	int lid_gpio ;
	int lid_close_value;
};

#endif
