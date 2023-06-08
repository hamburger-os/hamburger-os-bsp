import wave
import numpy as np
import matplotlib.pyplot as plt

with wave.open('test.wav', 'rb') as wav_file:
    # 获取wav文件参数
    num_channels = wav_file.getnchannels()
    sample_rate = wav_file.getframerate()
    num_frames = wav_file.getnframes()
    # 读取数据
    wav_data = wav_file.readframes(-1)

print("%d %d %d" % (num_channels, sample_rate, num_frames))

# 将wav数据转换为numpy数组
if num_channels == 1:
    # 单声道
    wav_data = np.frombuffer(wav_data, dtype=np.int16)
else:
    # 双声道
    wav_data = np.frombuffer(wav_data, dtype=np.int16)
    wav_data = wav_data.reshape((-1, num_channels))

# 计算时间轴
time_axis = np.arange(0, num_frames) * (1.0 / sample_rate)
print(len(time_axis))

# 绘制波形图
fig, ax = plt.subplots()
ax.plot(time_axis, wav_data[:, 0], color='b')
if num_channels == 2:
    ax.plot(time_axis, wav_data[:, 1], color='r')
ax.set_xlabel('Time (s)')
ax.set_ylabel('Amplitude')
ax.set_title('WAV Waveform')
plt.show()