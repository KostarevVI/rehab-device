class led{
  static int size;
  public int diameter;
  public int x_center;
  public int y_center;
  public boolean state;
  public int bin;
  public led(int d, int x,int y){
    size++;
    this.diameter=d;
    this.x_center=x;
    this.y_center=y;
    this.state=false;
  }
}
