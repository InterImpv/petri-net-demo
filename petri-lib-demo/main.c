#include "cJSON.h"
#include "petri.h"
#include "action.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_FUNCS 8
#define SYS_COUNT 3

static uint32_t LIGHTSW = 0;

char *jstr[SYS_COUNT] = {
    "{\"places\":[{\"name\":\"lt.EMPTY\",\"tokens\":1},{\"name\":\"lt.FULL\",\"tokens\":0},{\"name\":\"lt.OFF\",\"tokens\":1},{\"name\":\"lt.ON\",\"tokens\":0},{\"name\":\"lt.ENABLED\",\"tokens\":1}],\"transitions\":[{\"name\":\"lt.people>0\",\"inputs\":[{\"name\":\"lt.EMPTY\"}],\"outputs\":[{\"name\":\"lt.FULL\"}]},{\"name\":\"lt.people<1\",\"inputs\":[{\"name\":\"lt.FULL\"}],\"outputs\":[{\"name\":\"lt.EMPTY\"}]},{\"name\":\"lt.auto_off\",\"inputs\":[{\"name\":\"lt.ON\"},{\"name\":\"lt.EMPTY\"}],\"outputs\":[{\"name\":\"lt.OFF\"},{\"name\":\"lt.EMPTY\"}]},{\"name\":\"lt.turn_off\",\"inputs\":[{\"name\":\"lt.ON\"}],\"outputs\":[{\"name\":\"lt.OFF\"}]},{\"name\":\"lt.turn_on\",\"inputs\":[{\"name\":\"lt.OFF\"},{\"name\":\"lt.FULL\"},{\"name\":\"lt.ENABLED\"}],\"outputs\":[{\"name\":\"lt.ON\"},{\"name\":\"lt.FULL\"},{\"name\":\"lt.ENABLED\"}]}]}",
    "{\"places\":[{\"name\":\"cl.OFF\",\"tokens\":1},{\"name\":\"cl.IDLE\",\"tokens\":0},{\"name\":\"cl.ON\",\"tokens\":0},{\"name\":\"cl.EMPTY\",\"tokens\":1},{\"name\":\"cl.FULL\",\"tokens\":0},{\"name\":\"cl.ENABLED\",\"tokens\":1}],\"transitions\":[{\"name\":\"cl.init_standby\",\"inputs\":[{\"name\":\"cl.ENABLED\"},{\"name\":\"cl.OFF\"}],\"outputs\":[{\"name\":\"cl.ENABLED\"},{\"name\":\"cl.IDLE\"}]},{\"name\":\"cl.turn_off\",\"inputs\":[{\"name\":\"cl.IDLE\"}],\"outputs\":[{\"name\":\"cl.OFF\"}]},{\"name\":\"cl.turn_on\",\"inputs\":[{\"name\":\"cl.IDLE\"},{\"name\":\"cl.FULL\"}],\"outputs\":[{\"name\":\"cl.ON\"},{\"name\":\"cl.FULL\"}]},{\"name\":\"cl.to_standby\",\"inputs\":[{\"name\":\"cl.IDLE\"}],\"outputs\":[{\"name\":\"cl.ON\"}]},{\"name\":\"cl.people>0\",\"inputs\":[{\"name\":\"cl.EMPTY\"}],\"outputs\":[{\"name\":\"cl.FULL\"}]},{\"name\":\"cl.people<1\",\"inputs\":[{\"name\":\"cl.FULL\"}],\"outputs\":[{\"name\":\"cl.EMPTY\"}]}]}",
    "{\"places\":[{\"name\":\"fs.OFF\",\"tokens\":1},{\"name\":\"fs.ON\",\"tokens\":0},{\"name\":\"fs.T_NOM\",\"tokens\":1},{\"name\":\"fs.T_RISE\",\"tokens\":0},{\"name\":\"fs.ALARM\",\"tokens\":0},{\"name\":\"fs.TIMER\",\"tokens\":0},{\"name\":\"fs.WATER\",\"tokens\":0},{\"name\":\"fs.ENABLED\",\"tokens\":1}],\"transitions\":[{\"name\":\"fs.turn_on\",\"inputs\":[{\"name\":\"fs.ENABLED\"},{\"name\":\"fs.OFF\"}],\"outputs\":[{\"name\":\"fs.ENABLED\"},{\"name\":\"fs.ON\"}]},{\"name\":\"fs.turn_off\",\"inputs\":[{\"name\":\"fs.ON\"}],\"outputs\":[{\"name\":\"fs.OFF\"}]},{\"name\":\"fs.t<threshold\",\"inputs\":[{\"name\":\"fs.T_RISE\"}],\"outputs\":[{\"name\":\"fs.T_NOM\"}]},{\"name\":\"fs.t>=threshold\",\"inputs\":[{\"name\":\"fs.T_NOM\"},{\"name\":\"fs.ON\"}],\"outputs\":[{\"name\":\"fs.T_RISE\"},{\"name\":\"fs.ON\"}]},{\"name\":\"fs.dt>=threshold\",\"inputs\":[{\"name\":\"fs.T_RISE\"}],\"outputs\":[{\"name\":\"fs.ALARM\"},{\"name\":\"fs.TIMER\"}]},{\"name\":\"fs.fire_stop\",\"inputs\":[{\"name\":\"fs.ALARM\"},{\"name\":\"fs.TIMER\"}],\"outputs\":[{\"name\":\"fs.T_NOM\"}]},{\"name\":\"fs.smoke_detect\",\"inputs\":[{\"name\":\"fs.ALARM\"}],\"outputs\":[{\"name\":\"fs.WATER\"}]},{\"name\":\"fs.timer_timeout\",\"inputs\":[{\"name\":\"fs.ALARM\"},{\"name\":\"fs.TIMER\"}],\"outputs\":[{\"name\":\"fs.WATER\"}]},{\"name\":\"fs.timer_clear\",\"inputs\":[{\"name\":\"fs.TIMER\"}],\"outputs\":[]},{\"name\":\"fs.fire_done\",\"inputs\":[{\"name\":\"fs.WATER\"}],\"outputs\":[{\"name\":\"fs.T_NOM\"}]}]}"
};
// static const char *jstr_light = "";

int light_on(void *data)
{
    if (*(int32_t *)data) {
        printf("switch is now on ON\n");
        LIGHTSW = 1;
        return 0;
    }
    return -1;
}

int light_off(void *data)
{
    if (*(int32_t *)data) {
        printf("switch is now on OFF\n");
        LIGHTSW = 0;
        return 0;
    }
    return -1;
}

int main(void)
{
    srand(time(NULL));

    FILE *fp = NULL;
    if (!(fp = fopen("dump.csv", "w"))) {
        fprintf(stderr, "file open fail\n");
        return -1;
    }
    int ret = 0;

    const char *names[SYS_COUNT];
    names[0] = "LightSys";
    names[1] = "ClimateSys";
    names[2] = "FireSys";
    char buf[MAX_PLACES + 1] = {'\0'};

    pnet_t *sys[SYS_COUNT];

    for (uint32_t i = 0; i < SYS_COUNT; i++) {
        sys[i] = pnet_create();
        pnet_init(sys[i], names[i]);
        ret = pnet_deserialize_str(sys[i], jstr[i]);
        pnet_print(sys[i]);
        printf("%s %i\n\n", sys[i]->name, ret);
    }

    for (uint32_t i = 0; i < SYS_COUNT; i++) {
        pnet_dump_state_as_str(sys[i], buf);
        pnet_dump_to_hex(sys[i], fp);
        printf("%s = (%s)\n", sys[i]->name, buf);
    }

    for (uint32_t i = 0; i < SYS_COUNT; i++) {
        pnet_free(sys[i]);
    }

    fclose(fp);

    return 0;
}