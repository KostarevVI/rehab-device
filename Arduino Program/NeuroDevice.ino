#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Sector.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

#define POT_PIN A0
#define START_PIN 2
#define END_PIN 10

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// Interupts are disabled but in case: Irupt detection routine
//volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
//void dmpDataReady() {
//    mpuInterrupt = true;
//}

// Input params
int pot_value;
int g_z;
int g_x;
int g_y;

// Params for restriction of head movements by circle and for virtual location and size of sectors
const int r_sector = 833;
const int r_circle = 1400;
const uint32_t r_circle_sqr = 1960000;

// Array of sectors
Sector sector_matrix[9];

// Struct for each active sector's pulse width in micro seconds (us) and timer for PWM maintaining
struct SectorTimerPair {
    uint16_t timer: 9;
    uint8_t pin: 4;
};
uint32_t pwm_timer;

// Array of active sectors, can contain only 4 simultaneously
SectorTimerPair sector_timer_buffer[4];

// Array for matching of pin numbers with sectors
const uint8_t pin_match[9] = {6, 9, 8, 5, 2, 3, 4, 7, 10};

void insertionStructSort(SectorTimerPair a[], byte n);


void setup() {
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately

    // initialize device
    mpu.initialize();

     // verify connection
    mpu.testConnection();

    // load and configure the DMP
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // Calibration Time: generate offsets and calibrate our MPU6050
        mpu.CalibrateAccel(6);
        mpu.CalibrateGyro(6);
        mpu.PrintActiveOffsets();
        // turn on the DMP, now that it's ready
        mpu.setDMPEnabled(true);

        // uncomment to enable Arduino interrupt detection
//        attachInterrupt(digitalPinToInterrupt(12), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

    // configure pins for electrodes output and local positions 
    // sectors are located on a circle with radius r_circle virtual circle of sectors (electrodes) intensity map 
    for(int i = 0; i < 9; i++){
        if(i == 0){
          sector_matrix[i] = Sector(0, 0, r_sector, i);
        }else{
          sector_matrix[i] = Sector(int(cos(M_PI_4 * (i - 1)) * r_circle), int(sin(M_PI_4 * (i - 1)) * r_circle), r_sector, i);
        }
        pinMode(i + 2, OUTPUT);
        digitalWrite(i + 2, LOW);
    }
}


void loop() {
    // if programming failed, don't try to do anything    
    if (!dmpReady) return;

    Serial.print(g_z);
    Serial.print(" ");
    Serial.print(g_x);
    Serial.print(" ");
    Serial.print(g_y);
    Serial.print(" ");
    Serial.print(pot_value);
    Serial.print(" ");
    generateSignal(pot_value, g_x, g_y);
    Serial.println();

    // read a packet from FIFO (takes ~6400 us)
    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

      g_z = ypr[0] * 180/M_PI * 100;
      g_y = ypr[1] * 180/M_PI * 100;
      g_x = ypr[2] * -180/M_PI * 100;

      // restrict the area of head movments with circle
      if(abs(g_x) > r_circle)
          g_x = g_x/abs(g_x) * r_circle;
      if(abs(g_y) > r_circle)
          g_y = g_y/abs(g_y) * r_circle;

      uint32_t hypotenuse_sqr = long(g_x)*g_x + long(g_y)*g_y;
      
      if(hypotenuse_sqr > r_circle_sqr){
          float restrict_factor = float(r_circle)/sqrt(hypotenuse_sqr);
          g_x *= restrict_factor;
          g_y *= restrict_factor;
      }

      // read and map intensity value from potentiometer 
      pot_value = analogRead(POT_PIN);
      pot_value = map(pot_value, 0, 1023, 1, 105);  // Pulse width from 4 to 420 us (originally from 4 to 150 us for 14V output)
   }
}


// method for tripletPeriod generation
int generateSignal(int pot_value_mapped, float g_x, float g_y) {   
    uint8_t t_index = 0;  // Number of current elements in buffer
    uint16_t pwm_time = 0;  // Pulse width
    byte true_pin = 0;  // For output control with pin matching
    
    // Temporary variables for control of direct output
    byte TEMPB = B00000000;
    byte TEMPD = B00000000;
    
    for(int i = 0; i < 4; i++){
        sector_timer_buffer[i] = {255, 11};
    }
    
    for(int i = 0; i < 9; i++){
        pwm_time = sector_matrix[i].getPwmTime(g_x, g_y, pot_value_mapped);
                
        if(pwm_time != 0){
            true_pin = pin_match[i];
            sector_timer_buffer[t_index] = {pwm_time, true_pin};

            Serial.print(true_pin + String(" ") + pwm_time + String(" "));

            if(true_pin > 7){  
                TEMPB |= 1<<(true_pin - 8);
            } else {
                TEMPD |= 1<<true_pin;
            }

            t_index++;
        }
    }

//    Debug
//    prettyPrint(sector_timer_buffer);

    insertionStructSort(sector_timer_buffer, t_index);
    
//    Debug
//    prettyPrint(sector_timer_buffer);
//    Serial.println();
    
    for(uint8_t period = 0; period < 4; period++){
        if(period < 3){
            // Turn HIGH proper pins
            PORTB = TEMPB;
            PORTD = TEMPD;
            uint8_t t_iter = 0; // Iteration over the buffer
            uint32_t pwm_timer = micros(); // Set current time to timer
            
            while(t_iter < t_index){
                uint16_t timer_delay = sector_timer_buffer[t_iter].timer;
                
                while(micros() - pwm_timer < timer_delay);
                
                true_pin = sector_timer_buffer[t_iter].pin;
      
                // Turn LOW proper pins
                if(true_pin > 7){
                    PORTB &= ~(1<<(true_pin - 8));
                } else {
                    PORTD &= ~(1<<true_pin);
                }
    
                t_iter++;
            }
            PORTD = PORTD & B00000011;
            PORTB = PORTB & B11111000;
        
            while (micros() - pwm_timer < 5000);
        } else {
            while (micros() - pwm_timer < 7850);  // Time correction of all Serial.prints
        }
    }
}


// Arranges timers in order from small to big values
void insertionStructSort(SectorTimerPair a[], byte n){
  for (int i = 1; i < n; i++)
  {
    if (a[i].timer < a[i - 1].timer)
    { 
      int j = i - 1;
      SectorTimerPair x = a[i];    // Copy the sentinel, ready to sort the element
      while (j >= 0 && x.timer < a[j].timer)
      { // Find inserted in an orderly list
        a[j + 1] = a[j];
        j--; 
      }
      a[j + 1] = x; // Insert to the correct position
    }
  }
}


//// For debug of structSorter
//void prettyPrint(SectorTimerPair a[])
//{
//  Serial.print(" | ");
//  for (int i = 0; i < 4; i++)
//  {
//    Serial.print(a[i].timer+String("-")+a[i].pin+String("\t"));
//  }
//}
