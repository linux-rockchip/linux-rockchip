#include <adapter/frontend.h>

int antenna_reset(struct dibFrontend *fe)
{
    if (FE_ANT_FUNC_CHECK(fe, reset))
        return FE_ANT_FUNC_CALL(fe, reset)(fe);
    return DIB_RETURN_SUCCESS;
}

void antenna_release(struct dibFrontend *fe)
{
    if (FE_ANT_FUNC_CHECK(fe, release))
        FE_ANT_FUNC_CALL(fe, release)(fe);
}

void antenna_update(struct dibFrontend *fe, uint16_t v)
{
    if (FE_ANT_FUNC_CHECK(fe, update))
        FE_ANT_FUNC_CALL(fe, update)(fe,v);
}

const char *antenna_get_name(struct dibFrontend *fe)
{
    if (fe->antenna_info)
        return fe->antenna_info->name;
    return NULL;
}

struct dibDataBusClient * antenna_get_data_bus_client(struct dibFrontend *fe)
{
    if ((fe != NULL) && (fe->antenna_info != NULL))
        return &fe->antenna_info->data_bus_client;
    return NULL;
}
