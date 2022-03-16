import processing.serial.*;

Serial serial; 
String received;
String[] strArray = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};
String gyroString;
Boolean flag = false;
int intMatrix;

led[] led_matrix  = new led [9];
int y;
int x;
int startx=400;
int starty=40;

float g_z = 0;
float g_x = 0;
float g_y = 0;
int triplet = 0;


void setup()
{
  size(640, 250);
  surface.setTitle("Rehab Device interface");
  
  String port = Serial.list()[1];
  serial = new Serial(this, port, 115200);
  
  for(int i=0;i<9;i++){
    if(i%3==0 && i>0){
      x++;
      y=0;
    } //<>//
    led_matrix[i]=new led(30,startx+x*70,starty+y*70);
    y++;
  }
  //frame(30);
}

void draw() {
  clear();
  background(225);  
  
  if ( serial.available() > 0) { // If there any data
    received = serial.readStringUntil('\n'); // then read it
    println(received); // Print data to console
    if (received != null){
      strArray = split(received, ' ');
      if (flag == false && strArray != null && strArray[0].equals("0") && strArray[1].equals("0")){
        flag = true; 
      }
    }
  }
    
  if(flag){
    // Draw disabled leds
    for(int i=0;i<9; i++){
        fill(0,0,0);
        ellipse(led_matrix[i].x_center,led_matrix[i].y_center,60,60);
    }
    
    triplet = int(trim(strArray[3])) * 4;
    g_x = float(strArray[1])/100;
    g_y = float(strArray[2])/100;
    g_z = float(strArray[0])/100;
    
    fill(30);
    textSize(48);
    
    text("X: \t" + g_x + "°", 10, 50);
    text("Y: \t" + g_y + "°", 10, 100);
    text("Z: \t" + g_z + "°", 10, 150);
    text("Triplet: \t" + triplet + "µs", 10, 200);
    
    fill(30);
    textSize(28);
    text("Electrodes face forward", 310, 240);
    drawArrow(605, 200, 180, -90);
          
  
    // DISPLAY LEDS STATES    
    // Draw activate leds from array
    try{
    int index = 0;
    while(index * 2 + 4 < strArray.length - 1 && !strArray[index * 2 + 5].equals(null)){
      int color_intensity = int(log(float(trim(strArray[index * 2 + 5]))) * 70 * log(triplet)/log(420) / triplet * 255 + 0.5);
      //println(color_intensity);  // for debug
      fill(color_intensity, 0, 0);
      ellipse(led_matrix[int(trim(strArray[index * 2 + 4])) - 2].x_center, led_matrix[int(trim(strArray[index * 2 + 4])) - 2].y_center, 60, 60);// outOfBounds
      index++;
    }
    } catch(ArrayIndexOutOfBoundsException e) {
      println(e);
    }
    
    fill(255);
    ellipse(int(g_x * 7.071) + startx + 70, int(-g_y * 7.071) + starty + 70, 10, 10);
  } else {
    fill(30);
    textSize(48);
    text("Please, wait!", 160, 100);
    text("Calibrating IMU", 130, 150);
  }
  delay(10);
}


void drawArrow(int cx, int cy, int len, float angle){
  pushMatrix();
  translate(cx, cy);
  rotate(radians(angle));
  line(0,0,len, 0);
  line(len, 0, len - 8, -8);
  line(len, 0, len - 8, 8);
  popMatrix();
}
