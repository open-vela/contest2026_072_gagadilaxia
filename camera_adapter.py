import cv2
import yaml

class CameraAdapter:
    def __init__(self, config_path="config.yaml"):
        with open(config_path, "r") as f:
            self.config = yaml.safe_load(f)
        
        device = self.config["camera"]["device_path"]
        self.width = self.config["camera"]["width"]
        self.height = self.config["camera"]["height"]
        
        # 使用 GStreamer 管道读取虚拟摄像头
        gst_pipeline = (
            f"v4l2src device={device} ! "
            f"video/x-raw,width={self.width},height={self.height},framerate=30/1 ! "
            "videoconvert ! appsink"
        )
        self.cap = cv2.VideoCapture(gst_pipeline, cv2.CAP_GSTREAMER)
        
        if not self.cap.isOpened():
            raise Exception(f"无法打开摄像头: {device}")
        
        print(f"✅ 摄像头已打开: {device}")
    
    def get_frame(self):
        ret, frame = self.cap.read()
        return frame if ret else None
    
    def release(self):
        self.cap.release()
