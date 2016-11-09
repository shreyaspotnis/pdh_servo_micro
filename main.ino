// Code for the laser servo controller

// PIN configuration
// PIN# PIN_name        Connection          Type
// 5    WKUP            DS3                 Digital In
// 6    DAC             DS2                 Digital In
// 7    A5              S3_output_enable    Digital Out
// 8    A4              S2_curr_int         Digital Out
// 9    A3              S1_input            Digital Out
// 10   A2              S4_piezo_enable     Digital Out
// 11   A1              pz_out_buffer       Analog In
// 12   A0              transmission        Analog In
// 13   D0              S5_piezo_int        Digital Out
// 14   D1              S6_piezo_offset     Ditigal Out
// -----------------------------------------------------
// 15   D2              MOSI_SPI            Digital Out
// 17   D4              SCK_SPI             Digital Out
// 18   D5              SS_SPI              Digital Out
// 19   D6              LDAC_               Digital Out
// 20   D7              DS1                 Digital In

// Semi-automatic mode ensures that we run setup() before attempting to
// connect to the cloud.
SYSTEM_MODE(SEMI_AUTOMATIC);

int DS3 = WKP;
int DS2 = DAC;
int S3_output_enable = A5;
int S2_curr_int = A4;
int S1_input = A3;
int S4_piezo_enable = A2;
int pz_out_buffer = A1;
int transmission = A0;
int S5_piezo_int = D0;
int S6_piezo_offset = D1;
int DS1 = D7;
int LDAC_ = D6;
int SS_SPI = D5;

// Global variables
bool s2_state = LOW;
bool s5_state = LOW;

int dac_center = 32768; // center value around which to scan
int dac_word = dac_center;  // actual dac value written
int dac_scan_range = 10000;  // scan range
int dac_scan_step = 10;
int dac_scan_value = -dac_scan_range;

void setup() {
    // Read the analog pins once to set them to input mode
    analogRead(transmission);
    analogRead(pz_out_buffer);

    // Set the manual toggle switches to input mode
    pinMode(DS1, INPUT);
    pinMode(DS2, INPUT);
    pinMode(DS3, INPUT);

    // set the analog switch lines to digital output
    pinMode(S1_input, OUTPUT);
    pinMode(S2_curr_int, OUTPUT);
    pinMode(S3_output_enable, OUTPUT);

    pinMode(S4_piezo_enable, OUTPUT);
    pinMode(S5_piezo_int, OUTPUT);
    pinMode(S6_piezo_offset, OUTPUT);

    // Set the DAC latch (LDAC_) and chip select (SS_SPI) pins to ditigal output
    pinMode(LDAC_, OUTPUT);
    pinMode(SS_SPI, OUTPUT);
    digitalWrite(LDAC_, HIGH); // The serial register is not latched with LDAC_ is HIGH
    digitalWrite(SS_SPI, HIGH);  // HIGH disables the DAC, it ignores the clock
                                 // and data lines

    // Start with all the analog switches closed
    digitalWrite(S1_input, LOW);
    digitalWrite(S2_curr_int, s2_state);
    digitalWrite(S3_output_enable, LOW);
    digitalWrite(S4_piezo_enable, LOW);
    digitalWrite(S5_piezo_int, s5_state);
    digitalWrite(S6_piezo_offset, LOW);


    SPI1.setBitOrder(MSBFIRST);
    SPI1.setClockSpeed(10, MHZ);
    SPI1.begin();

    // Set the DAC output to midscale
    update_dac(dac_word);

    // Register the dac_word variable so that it can be accessed from the cloud
    // Particle.variable("dac_word", dac_word);

    Particle.connect();
}

void update_dac(uint16_t dac_word_) {
    uint8_t msbyte = dac_word_ >> 8;
    uint8_t lsbyte = dac_word_ && 0xFF;

    //digitalWrite(LDAC_, LOW);
    digitalWrite(SS_SPI, LOW);
    SPI1.transfer(msbyte);
    SPI1.transfer(lsbyte);
    digitalWrite(SS_SPI, HIGH);
    digitalWrite(LDAC_, LOW);
    digitalWrite(LDAC_, HIGH);
}

void loop() {
    update_dac(dac_word);
    if(digitalRead(DS3)) {
        dac_scan_value += dac_scan_step;
        if(dac_scan_value >= dac_scan_range)
            dac_scan_value = -dac_scan_range;
        dac_word = dac_center + dac_scan_value;
    }
    else {
        dac_word = dac_center;
    }

    bool ds1_state = digitalRead(DS1);
    if(s5_state != ds1_state) {
        s5_state = ds1_state;
        if(ds1_state)
            Particle.connect();
        else
            Particle.disconnect();
    }

    bool ds2_state = digitalRead(DS2);
    if(s2_state != ds2_state) {
        s2_state = ds2_state;
        digitalWrite(S2_curr_int, s2_state);
    }

}
