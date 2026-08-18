import cv2
from camera_adapter import CameraAdapter


class BehaviorAnalyzer:
    def __init__(self, config_path="config.yaml"):
        self.camera = CameraAdapter(config_path)
        self.frame_count = 0

    def analyze(self):
        print("🎥 开始视觉分析，按 q 退出")

        while True:
            frame = self.camera.get_frame()

            if frame is None:
                print("⚠️ 读取帧失败")
                break

            self.frame_count += 1

            print(
                f"帧 {self.frame_count}: "
                f"尺寸 {frame.shape}"
            )

            cv2.imshow("Behavior Analysis", frame)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                break

        self.camera.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    analyzer = BehaviorAnalyzer()
    analyzer.analyze()
