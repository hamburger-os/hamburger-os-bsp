import wave
import struct

# 打开双通道WAV文件
wav_original = wave.open('test.wav', 'rb')
# 读取WAV文件的参数
channels = wav_original.getnchannels()
sample_width = wav_original.getsampwidth()
frame_rate = wav_original.getframerate()
frames = wav_original.getnframes()

# 创建新的WAV文件
wav_new = wave.open('output.wav', 'wb')
wav_new.setnchannels(channels)
wav_new.setsampwidth(sample_width)
wav_new.setframerate(frame_rate)

# 读取每个frame的数据
for i in range(frames):
    # 读取双通道WAV文件的数据
    data = wav_original.readframes(1)
    # 分离左右通道数据
    left = struct.unpack("<h", data[:2])[0]
    right = struct.unpack("<h", data[2:])[0]

    # 将右通道数据幅值为0
    if channels == 2:
        left = 0

    # 将左右通道数据合并，并写入新的WAV文件
    merged_data = struct.pack("<h", left) + struct.pack("<h", right)
    wav_new.writeframes(merged_data)

# 关闭文件
wav_original.close()
wav_new.close()
