bool Launch_Switch; //Mux 1, Pin 0
bool TLI_Switch; //Mux 1, Pin 1
bool BOOSTER_Go; //Mux 1, Pin 2
bool BOOSTER_NoGo; //Mux 1, Pin 3
bool RETRO_Go; //Mux 1, Pin 4
bool RETRO_NoGo; //Mux 1, Pin 5
bool FDO_Go; //Mux 1, Pin 6
bool FDO_NoGo; //Mux 1, Pin 7
bool GUIDO_Go; //Mux 2, Pin 0
bool GUIDO_NoGo; //Mux 2, Pin 1
bool Surgeon_Go; //Mux 2, Pin 2
bool Surgeon_NoGo; //Mux 2, Pin 3
bool EECOM_Go; //Mux 2, Pin 4
bool EECOM_NoGo; //Mux 2, Pin 5
bool GNC_Go; //Mux 2, Pin 6
bool GNC_NoGo; //Mux 2, Pin 7
bool TELMU_Go; //Mux 3, Pin 0
bool TELMU_NoGo; //Mux 3, Pin 1
bool Control_Go; //Mux 3, Pin 2
bool Control_NoGo; //Mux 3, Pin 3
bool Go_On;
bool Go_Off;
bool NoGo_Off;
bool Poll_Ready_Flag;
bool Poll_Flag;
bool All_Systems_Go;
bool Comm_Switch; //Mux 3, Pin 4
bool Poll_Switch; //Mux 3, Pin 5
bool Abort_Switch; //Mux 3, Pin 6
bool Landing_Switch; //Mux 3, Pin 7
bool SCE_Switch; //D3, wakeup pin
bool Power_LED = true; //Mux 4, Pin 2
bool Comm_LED = false; //Mux 4, Pin 3
bool Status_Red_LED = false; //Mux 4, Pin 4
bool Status_Yellow_LED = false; //Mux 4, Pin 5
bool Status_Green_LED = false; //Mux 4, Pin 6
bool Abort_LED = false; //Mux 4, Pin 7
bool Sleep_Mode_Flag = false;

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
int TotalChannels = 8;
#define Mux_Address_A D0
#define Mux_Address_B D1
#define Mux_Address_C D2
#define SCE_Switch_Pin D3
#define Mux1_Pin A0
#define Mux2_Pin A1
#define Mux3_Pin A2
#define Mux4_Pin A3
int Mux_A;      //Value to write to Address pin A
int Mux_B;      //Value to write to Address pin B
int Mux_C;      //Value to write to Address pin C

Timer timer_Sleep_Mode(300000, Sleep_Mode, true); //5 minute timer to put the device to sleep

SYSTEM_THREAD(ENABLED);

void setup() {
    //Made all these variables so I could test that the switches were working properly
    /*
    Particle.variable("Launch_Switch", Launch_Switch);
    Particle.variable("TLI_Switch", TLI_Switch);
    Particle.variable("BOOSTER_Go", BOOSTER_Go);
    Particle.variable("BOOSTER_NoGo", BOOSTER_NoGo);
    Particle.variable("RETRO_Go", RETRO_Go);
    Particle.variable("RETRO_NoGo", RETRO_NoGo);
    Particle.variable("FDO_Go", FDO_Go);
    Particle.variable("FDO_NoGo", FDO_NoGo);
    Particle.variable("GUIDO_Go", GUIDO_Go);
    
    Particle.variable("GUIDO_NoGo", GUIDO_NoGo);
    Particle.variable("Surgeon_Go", Surgeon_Go);
    Particle.variable("Surgeon_NoGo", Surgeon_NoGo);
    Particle.variable("EECOM_Go", EECOM_Go);
    Particle.variable("EECOM_NoGo", EECOM_NoGo);
    Particle.variable("GNC_Go", GNC_Go);
    Particle.variable("GNC_NoGo", GNC_NoGo);
    Particle.variable("TELMU_Go", TELMU_Go);
    Particle.variable("TELMU_NoGo", TELMU_NoGo);
    
    Particle.variable("Control_Go", Control_Go);
    Particle.variable("Control_NoGo", Control_NoGo);
    Particle.variable("Comm_Switch", Comm_Switch);
    Particle.variable("Poll_Switch", Poll_Switch);
    Particle.variable("Abort_Switch", Abort_Switch);
    Particle.variable("Landing_Switch", Landing_Switch);
    Particle.variable("SCE_Switch", SCE_Switch);
    */
    
    // Prepare address and read pins
    pinMode(Mux_Address_A, OUTPUT);
    pinMode(Mux_Address_B, OUTPUT);
    pinMode(Mux_Address_C, OUTPUT);
    pinMode(Mux1_Pin, INPUT_PULLUP);
    pinMode(Mux2_Pin, INPUT_PULLUP);
    pinMode(Mux3_Pin, INPUT_PULLUP);
    pinMode(SCE_Switch_Pin, INPUT_PULLUP);
    pinMode(Mux4_Pin, OUTPUT);
    
    timer_Sleep_Mode.start();
}

void loop() {
    //Toggle Particle Cloud connection based on Comm Switch position
    if (Comm_Switch) {
        Particle.disconnect();
    }
        else {
            Particle.connect();
        }
    
    //Toggle Comm LED based on particle cloud connection status
    if (Particle.connected()) {
        Comm_LED = true;
    }
        else {
            Comm_LED = false;
        }
    
    //Toggle Abort LED based on Abort Switch position
    if (Abort_Switch) {
        Abort_LED = false;
    }
        else {
            Abort_LED = true;
            All_Systems_Go = false;
            Poll_Ready_Flag = false;
            Poll_Flag = false;
        }

    //Initiate polling if conditions met and poll switch turned on
    if (Poll_Ready_Flag and Poll_Switch==false) {
        Poll_Flag = true;
        Status_Yellow_LED = true;
    }
        //Stop polling and shutoff LEDs whenever the polling switch is turned off
        else if (Poll_Switch or Abort_Switch==false) {
            Poll_Flag = false;
            Status_Red_LED = false;
            Status_Yellow_LED = false;
            Status_Green_LED = false;
            All_Systems_Go = false;
        }

    //Turn on Red LED if any of the NoGos are switched on. Turn on Green LED if all of the Gos are switched on.
    if (Poll_Flag and NoGo_Off==false) {
        Status_Red_LED = true;
        Status_Green_LED = false;
        All_Systems_Go = false;
    }
        else if (Poll_Flag and Go_On) {
            Status_Green_LED = true;
            Status_Red_LED = false;
            All_Systems_Go = true;
        }
        
        else if (Poll_Flag and Go_On==false) {
            Status_Green_LED = false;
            Status_Red_LED = false;
            All_Systems_Go = false;
        }

    //When all conditions are met, arm all switches and allow for one action to occur before requiring a new poll
    if (All_Systems_Go) {
        if (Launch_Switch == false) {
            Particle.publish("Launch", "Audio", 60, PRIVATE);
            All_Systems_Go = false;
            Poll_Flag = false;
            Poll_Ready_Flag = false;
            Status_Red_LED = false;
            Status_Yellow_LED = false;
            Status_Green_LED = false;
        }
        else if (TLI_Switch == false) {
            Particle.publish("Launch", "NoGo", 60, PRIVATE);
            All_Systems_Go = false;
            Poll_Flag = false;
            Poll_Ready_Flag = false;
            Status_Red_LED = false;
            Status_Yellow_LED = false;
            Status_Green_LED = false;
        }
        else if (Landing_Switch == false) {
            Particle.publish("Landing", "Audio", 60, PRIVATE);
            All_Systems_Go = false;
            Poll_Flag = false;
            Poll_Ready_Flag = false;
            Status_Red_LED = false;
            Status_Yellow_LED = false;
            Status_Green_LED = false;
        }
    }
    
    //Determine group status of all Go/NoGo switches
    if (BOOSTER_Go and RETRO_Go and FDO_Go and GUIDO_Go and Surgeon_Go and EECOM_Go and GNC_Go and TELMU_Go and Control_Go) {
        Go_On = false;
        Go_Off = true;
    }
        else if(BOOSTER_Go==false and RETRO_Go==false and FDO_Go==false and GUIDO_Go==false and Surgeon_Go==false and EECOM_Go==false and GNC_Go==false and TELMU_Go==false and Control_Go==false) {
            Go_On = true;
            Go_Off = false;
        }
        else {
            Go_On = false;
            Go_Off = false;
        }
    if (BOOSTER_NoGo and RETRO_NoGo and FDO_NoGo and GUIDO_NoGo and Surgeon_NoGo and EECOM_NoGo and GNC_NoGo and TELMU_NoGo and Control_NoGo) {
        NoGo_Off = true;
    }
        else {
            NoGo_Off = false;
        }
    
    //Determine if polling conditions are met
    if (Launch_Switch and TLI_Switch and Poll_Switch and Abort_Switch and Landing_Switch and Go_Off and NoGo_Off and Comm_LED) {
        Poll_Ready_Flag = true;
    }
        else {
            Poll_Ready_Flag = false;
        }
    
    //Select each pin and read the current value
    for(int i=0; i<TotalChannels; i++){
        Mux_A = bitRead(i,0); //Take first bit from binary value of i channel.
        Mux_B = bitRead(i,1); //Take second bit from binary value of i channel.
        Mux_C = bitRead(i,2); //Take third bit from value of i channel.
        //Write address to muxes and read values
        digitalWrite(Mux_Address_A, Mux_A);
        digitalWrite(Mux_Address_B, Mux_B);
        digitalWrite(Mux_Address_C, Mux_C);
        
        //Switch Case to correlate current pins to correct variables
        switch (i) {
            case 0:
                Launch_Switch = digitalRead(Mux1_Pin);
                GUIDO_Go = digitalRead(Mux2_Pin);
                TELMU_Go = digitalRead(Mux3_Pin);
                //Mux 4, Pin 0 not used
                break;
                
            case 1:
                TLI_Switch = digitalRead(Mux1_Pin);
                GUIDO_NoGo = digitalRead(Mux2_Pin);
                TELMU_NoGo = digitalRead(Mux3_Pin);
                //Mux 4, Pin 1 not used
                break;
                
            case 2:
                BOOSTER_Go = digitalRead(Mux1_Pin);
                Surgeon_Go = digitalRead(Mux2_Pin);
                Control_Go = digitalRead(Mux3_Pin);
                digitalWrite(Mux4_Pin, Power_LED);
                delay(1);
                digitalWrite(Mux4_Pin, false);
                break;
                
            case 3:
                BOOSTER_NoGo = digitalRead(Mux1_Pin);
                Surgeon_NoGo = digitalRead(Mux2_Pin);
                Control_NoGo = digitalRead(Mux3_Pin);
                digitalWrite(Mux4_Pin, Comm_LED);
                delay(1);
                digitalWrite(Mux4_Pin, false);
                break;
                
            case 4:
                RETRO_Go = digitalRead(Mux1_Pin);
                EECOM_Go = digitalRead(Mux2_Pin);
                Comm_Switch = digitalRead(Mux3_Pin);
                digitalWrite(Mux4_Pin, Status_Red_LED);
                delay(1);
                digitalWrite(Mux4_Pin, false);
                break;
                
            case 5:
                RETRO_NoGo = digitalRead(Mux1_Pin);
                EECOM_NoGo = digitalRead(Mux2_Pin);
                Poll_Switch = digitalRead(Mux3_Pin);
                digitalWrite(Mux4_Pin, Status_Yellow_LED);
                delay(1);
                digitalWrite(Mux4_Pin, false);
                break;
                
            case 6:
                FDO_Go = digitalRead(Mux1_Pin);
                GNC_Go = digitalRead(Mux2_Pin);
                Abort_Switch = digitalRead(Mux3_Pin);
                digitalWrite(Mux4_Pin, Status_Green_LED);
                delay(1);
                digitalWrite(Mux4_Pin, false);
                break;
                
            case 7:
                FDO_NoGo = digitalRead(Mux1_Pin);
                GNC_NoGo = digitalRead(Mux2_Pin);
                Landing_Switch = digitalRead(Mux3_Pin);
                digitalWrite(Mux4_Pin, Abort_LED);
                delay(1);
                digitalWrite(Mux4_Pin, false);
                break;
                
            default:
                break;
        }

        if (Sleep_Mode_Flag) {
            System.sleep(SCE_Switch_Pin,CHANGE);
            timer_Sleep_Mode.start();
            Sleep_Mode_Flag = false;
        }
    }
}

void Sleep_Mode()
{
    Sleep_Mode_Flag = true;
}