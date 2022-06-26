/*
 *  VTC Heater Control Library
 *
 *  Created on: Dec 28, 2020
 *
 *  Author: Coskun ERGAN
 */

typedef union Zone_Flag_s
{
    unsigned char Value;
    struct ZoneFlagBits
    {
        unsigned char A: 1;
        unsigned char B: 1;
        unsigned char C: 1;
        unsigned char D: 1;
        unsigned char E: 1;
        unsigned char F: 1;
        unsigned char Dual_1: 1;
        unsigned char Dual_2: 1;
    } bits;
} Zone_Flag_t;

typedef enum
{
    WORK_OFF = 0,
    WORK_PRE_HEAT,
    WORK_HEAT_UP,
    WORK_HEAT_DOWN
} Work_State_t;

static const unsigned int  PreHeat_Time[] = {0, 15, 15, 15, 20, 20, 20, 60, 90, 900, 1800};
static const unsigned char  HeatUp_Time[] =   {0, 2,  4,  6,  8,  10, 15, 10, 15, 20, 30};
static const unsigned char  HeatDown_Time[] = {0, 62, 60, 47, 44, 42, 40, 20, 15, 10, 10};

Zone_Flag_t  ZonePower;

extern unsigned char Zone_Step[];
extern unsigned char Zone_RunTime[NUMBER_OF_ZONE];
extern unsigned int Total_Power_Limit;

unsigned char SyncronizedZones = RESET;
unsigned char ZoneProccesArray[NUMBER_OF_ZONE] = {0, 0, 0, 0};
unsigned char LastSuspendZone = 0;

void Vtc_Procces(void)  // per 1 seconds
{
    static unsigned int   work_timer[NUMBER_OF_ZONE] = {0, 0, 0, 0};
    static Work_State_t   work_state[NUMBER_OF_ZONE];
    static unsigned char   Zone_Step_Mem[NUMBER_OF_ZONE] = {0, 0, 0, 0};
    unsigned char zone, activezones, i;

    //------------------------
    //------------------------
    //------------------------
    if(Total_Power_Limit <= 3600)
    {
        activezones = 0;
        for(zone = 0; zone < NUMBER_OF_ZONE; zone++)
        {
            if(Zone_Step[zone] > 0)
            {
                activezones++;
                if(Zone_Step_Mem[zone] != Zone_Step[zone])
                {
                    ZoneProccesArray[3] = ZoneProccesArray[2];
                    ZoneProccesArray[2] = ZoneProccesArray[1];
                    ZoneProccesArray[1] = ZoneProccesArray[0];
                    ZoneProccesArray[0] = zone;
                    SyncronizedZones = RESET;
                }
            }
        }
        if((activezones == 3) && (SyncronizedZones == RESET))
        {
            ZonePower.Value = 0; // All power off
            work_timer[ZoneProccesArray[2]] = 0;
            work_state[ZoneProccesArray[2]] = WORK_HEAT_UP; // turn OFF
            work_timer[ZoneProccesArray[1]] = 0;
            work_state[ZoneProccesArray[1]] = WORK_HEAT_DOWN; // turn ON
            work_timer[ZoneProccesArray[0]] = 0;
            work_state[ZoneProccesArray[0]] = WORK_HEAT_DOWN; // turn ON
            SyncronizedZones = SET;
        }
        else if((activezones == 4) && (SyncronizedZones == RESET))
        {
            ZonePower.Value = 0; // All power off
            work_timer[ZoneProccesArray[3]] = 0;
            work_state[ZoneProccesArray[3]] = WORK_HEAT_UP; // turn OFF
            work_timer[ZoneProccesArray[2]] = 0;
            work_state[ZoneProccesArray[2]] = WORK_HEAT_UP; // turn OFF
            work_timer[ZoneProccesArray[1]] = 0;
            work_state[ZoneProccesArray[1]] = WORK_HEAT_DOWN; // turn ON
            work_timer[ZoneProccesArray[0]] = 0;
            work_state[ZoneProccesArray[0]] = WORK_HEAT_DOWN; // turn ON
            SyncronizedZones = SET;
        }
    }
    //------------------------
    //------------------------
    //------------------------
    for(zone = 0; zone < NUMBER_OF_ZONE; zone++)
    {
        if(Zone_Step[zone] == 0)
        {
            Zone_Step_Mem[zone] = Zone_Step[zone];
            ZonePower.Value &= ~(1 << zone); // power off
            work_timer[zone] = 0;
            work_state[zone] = WORK_OFF;
        }
        else if(work_timer[zone] == 0)
        {
            Zone_Step_Mem[zone] = Zone_Step[zone];
            switch(work_state[zone])
            {
                case WORK_OFF:
                    if((Zone_RunTime[zone] == 180) || (Zone_RunTime[zone] >= PreHeat_Time[Zone_Step[zone]]))
                    {
                        work_state[zone] = WORK_HEAT_DOWN;
                    }
                    else
                    {
                        work_state[zone] = WORK_PRE_HEAT;
                        if(PreHeat_Time[Zone_Step[zone]] > Zone_RunTime[zone])
                        {
                            work_timer[zone] = PreHeat_Time[Zone_Step[zone]] - Zone_RunTime[zone];
                        }
                        else
                        {
                            work_timer[zone] = PreHeat_Time[Zone_Step[zone]];
                        }
                        ZonePower.Value |= (1 << zone); // power on
                    }
                    break;
                case WORK_PRE_HEAT:
                    work_state[zone] = WORK_HEAT_DOWN;
                    work_timer[zone] = HeatDown_Time[Zone_Step[zone]];
                    ZonePower.Value &= ~(1 << zone); // power off
                    break;
                case WORK_HEAT_DOWN:
                    if(SyncronizedZones == RESET)
                    {
                        work_state[zone] = WORK_HEAT_UP;
                        work_timer[zone] = HeatUp_Time[Zone_Step[zone]];
                        ZonePower.Value |= (1 << zone); // power on
                    }
                    else
                    {
                        activezones = 0;
                        for(i = 0; i < NUMBER_OF_ZONE; i++)
                        {
                            if(work_state[i] == WORK_HEAT_UP)
                            {
                                activezones++;
                            }
                        }
                        if(activezones >= 2)
                        {
                            if(LastSuspendZone != zone)
                            {
                                work_timer[zone] = 1;
                                LastSuspendZone = zone;
                            }
                            else
                            {
                                work_timer[zone] = 2;
                            }
                        }
                        else
                        {
                            work_state[zone] = WORK_HEAT_UP;
                            work_timer[zone] = HeatUp_Time[Zone_Step[zone]];
                            ZonePower.Value |= (1 << zone); // power on
                        }
                    }
                    break;
                case WORK_HEAT_UP:
                    work_state[zone] = WORK_HEAT_DOWN;
                    work_timer[zone] = HeatDown_Time[Zone_Step[zone]];
                    ZonePower.Value &= ~(1 << zone); // power off
                    break;
            }
        }
        else
        {
            work_timer[zone]--;
            if(Zone_Step_Mem[zone] != Zone_Step[zone])
            {
                if(Zone_Step_Mem[zone] > Zone_Step[zone])
                {
                    if(work_state[zone] == WORK_PRE_HEAT)
                    {
                        if((PreHeat_Time[Zone_Step_Mem[zone]] - work_timer[zone]) >= PreHeat_Time[Zone_Step[zone]])
                        {
                            work_state[zone] = WORK_HEAT_UP; // turn off
                            work_timer[zone] = 0;
                        }
                        else
                        {
                            work_timer[zone] -= PreHeat_Time[Zone_Step_Mem[zone]] - PreHeat_Time[Zone_Step[zone]];
                        }
                    }
                    else
                    {
                        work_state[zone] = WORK_HEAT_UP; // turn off
                        work_timer[zone] = 0;
                    }
                }
                else
                {
                    if(work_state[zone] == WORK_PRE_HEAT)
                    {
                        work_timer[zone] +=  PreHeat_Time[Zone_Step[zone]] - PreHeat_Time[Zone_Step_Mem[zone]];
                    }
                    else
                    {
                        if((Zone_RunTime[zone] == 180) || (Zone_RunTime[zone] >= PreHeat_Time[Zone_Step[zone]]))
                        {
                            work_state[zone] = WORK_HEAT_DOWN;// turn on
                            work_timer[zone] = 0;
                        }
                        else
                        {
                            work_state[zone] = WORK_OFF;  // turn on
                            work_timer[zone] = 0;
                        }
                    }
                }
                Zone_Step_Mem[zone] = Zone_Step[zone];
            }
        }
    }
    //-----------------------------
    Serial.printf("Zone ABCD: %d - %d - %d - %d\n",
    (unsigned char)ZonePower.bits.A,
    (unsigned char)ZonePower.bits.B,
    (unsigned char)ZonePower.bits.C,
    (unsigned char)ZonePower.bits.D);

    //----------------------------
}
