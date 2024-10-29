void setup() {
  // put your setup code here, to run once:

}

void loop() {
  Serial.begin(115200);
  myCAM.begin();
  myCAM.takePicture(CAM_IMAGE_MODE_WQXGA2,CAM_IMAGE_PIX_FMT_JPG);
}
