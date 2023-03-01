

import sensor, image, time
from pyb import UART

#qiu_threshold   = ((70, 100, 8, 119, -35, 123))
red_threshold   =(72, 100, 14, 63, -11, 29)
#red_threshold=(88, 100, -33, -13, -18, 11)
#green_threshold = (88, 100, -36, -9, -19, 13)
green_threshold =(62, 100, -70, -7, -22, 49)
#(81, 100, -70, -7, -22, 49) (84, 100, -39, -6, -20, 8)
#设置绿色的阈值，括号里面的数值分别是L A B 的最大值和最小值（minL, maxL, minA,
# maxA, minB, maxB），LAB的值在图像左侧三个坐标图中选取。如果是灰度图，则只需
#设置（min, max）两个数字即可。

# You may need to tweak the above settings for tracking green things...
# Select an area in the Framebuffer to copy the color settings.

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # use RGB565.
sensor.set_framesize(sensor.QQVGA) # use QQVGA for speed.
sensor.skip_frames(200)
sensor.set_auto_whitebal(False) # turn this off.
sensor.set_auto_gain(False)#自动增益开
roi1=(62,52,31,26)
#sensor.set_windowing(roi1)
#关闭白平衡。白平衡是默认开启的，在颜色识别中，需要关闭白平衡。
clock = time.clock() # Tracks FPS.
uart = UART(3, 9600, timeout_char=10)
# 注意！与find_qrcodes不同，find_apriltags 不需要软件矫正畸变就可以工作。

# 注意，输出的姿态的单位是弧度，可以转换成角度，但是位置的单位是和你的大小有关，需要等比例换算

# f_x 是x的像素为单位的焦距。对于标准的OpenMV，应该等于2.8/3.984*656，这个值是用毫米为单位的焦距除以x方向的感光元件的长度，乘以x方向的感光元件的像素（OV7725）
# f_y 是y的像素为单位的焦距。对于标准的OpenMV，应该等于2.8/2.952*488，这个值是用毫米为单位的焦距除以y方向的感光元件的长度，乘以y方向的感光元件的像素（OV7725）

# c_x 是图像的x中心位置
# c_y 是图像的y中心位置

f_x = (2.8 / 3.984) * 160 # 默认值
f_y = (2.8 / 2.952) * 120 # 默认值
c_x = 160 * 0.5 # 默认值(image.w * 0.5)
c_y = 120 * 0.5 # 默认值(image.h * 0.5)

def degrees(radians):
    return (180 * radians) / math.pi
ROI=(80,30,5,5)
tag_families = 0
#tag_families |= image.TAG16H5 # comment out to disable this family
#tag_families |= image.TAG25H7 # comment out to disable this family
#tag_families |= image.TAG25H9 # comment out to disable this family
#tag_families |= image.TAG36H10 # comment out to disable this family
tag_families |= image.TAG36H11 # comment out to disable this family (default family)
#tag_families |= image.ARTOOLKIT # comment out to disable this family


while(True):


    #find_blobs(thresholds, invert=False, roi=Auto),thresholds为颜色阈值，
    #是一个元组，需要用括号［ ］括起来。invert=1,反转颜色阈值，invert=False默认
    #不反转。roi设置颜色识别的视野区域，roi是一个元组， roi = (x, y, w, h)，代表
    #从左上顶点(x,y)开始的宽为w高为h的矩形区域，roi不设置的话默认为整个图像视野。
    #这个函数返回一个列表，[0]代表识别到的目标颜色区域左上顶点的x坐标，［1］代表
    #左上顶点y坐标，［2］代表目标区域的宽，［3］代表目标区域的高，［4］代表目标
    #区域像素点的个数，［005］代表目标区域的中心点x坐标，［6］代表目标区域中心点y坐标，
    #［7］代表目标颜色区域的旋转角度（是弧度值，浮点型，列表其他元素是整型），
    #［8］代表与此目标区域交叉的目标个数，［9］代表颜色的编号（它可以用来分辨这个
    #区域是用哪个颜色阈值threshold识别出来的）。
    #if uart.any():
     #  a = uart.readline().decode().strip()
      # if   a== '1' :
       #  roi_=(185,66,60,59)
       #elif a== '2':
       #  roi_=(113,84,38,34)
       #elif a== '3':
        # print('4')
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot()#.lens_corr(strength = 1.8, zoom = 1.0)
    img.draw_rectangle(roi1)
   # img.draw_rectangle(roi1)
    blobs = img.find_blobs([red_threshold],x_stride=2,y_stride=2,roi=roi1,merge=True,pixels_threshold=5)
    if blobs:
    #如果找到了目标颜色
        for b in blobs:
        #迭代找到的目标颜色区域
            # Draw a rect around the blob.
            #img.draw_rectangle(b[0:4]) # rect
            #用矩形标记出目标颜色区域
            #if (b[4] >= 35):

            img.draw_cross(b[5], b[6]) # cx, cy
            #在目标颜色区域的中心画十字形标记
            uart.write('1')
            print('1')
            #uart.write("w%d\n"%b[6])
            #输出目标物体中心坐标
            #print("W%d %d"%(b[5],b[6]))

    blobs = img.find_blobs([green_threshold],x_stride=2,y_stride=2,roi=roi1,merge=True,pixels_threshold=3)
    if blobs:
            #如果找到了目标颜色
                for b in blobs:
                #迭代找到的目标颜色区域
                    # Draw a rect around the blob.
                    #img.draw_rectangle(b[0:4]) # rect
                    #用矩形标记出目标颜色区域
                    #if (b[4] >= 35):

                    img.draw_cross(b[5], b[6]) # cx, cy
                    #在目标颜色区域的中心画十字形标记
                    uart.write('3')
                    #uart.write("w%d\n"%b[6])
                    #输出目标物体中心坐标
                    print('3')

    for tag in img.find_apriltags(families=tag_families): # 默认为TAG36H11
        img.draw_rectangle(tag.rect(), color = (255, 0, 0))
        img.draw_cross(tag.cx(), tag.cy(), color = (0, 255, 0))
        #print_args = (tag.x_translation(), tag.y_translation(), tag.z_translation(),   )
        # 位置的单位是未知的，旋转的单位是角度

        #uart.write('2')
        #if tag.z_translation()<=?
        uart.write('2')
        print('2')
        #print( tag.z_translation())
    #print(clock.fps())




