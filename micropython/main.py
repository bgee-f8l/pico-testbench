from machine import Pin, UART, I2C
from time import sleep
from device import ina219
from device import ssd1306

# setup uart
uart0 = UART(0, baudrate=115200, tx=Pin(0), rx=Pin(1), bits=8, parity=None, stop=1)

# setup gpio
pin  = Pin("LED", Pin.OUT)
sense = Pin(2, Pin.OUT)

# setup I2C
i2c0 = I2C(0, scl=Pin(5), sda=Pin(4))
i2c1 = I2C(1, scl=Pin(7), sda=Pin(6))
print("I2C0 Bus Scan: ", i2c0.scan(), "\n")
print("I2C1 Bus Scan: ", i2c1.scan(), "\n")

# uart buffer
txData = b'Starting Power Monitor\n\r'
uart0.write(txData)

# setup INA219
# I2C0
row_vgh = ina219.INA219(i2c0, 0x40)
row_vgh.set_calibration_16V_400mA()
col_vsh = ina219.INA219(i2c0, 0x41)
col_vsh.set_calibration_16V_400mA()
sys_5v = ina219.INA219(i2c0, 0x44)
sys_5v.set_calibration_16V_400mA()
sys_3v3 = ina219.INA219(i2c0, 0x45)
sys_3v3.set_calibration_16V_400mA()

# setup oled
oled0 = ssd1306.SSD1306_I2C(128, 32, i2c0)
oled0.fill(0)
oled0.show()

oled1 = ssd1306.SSD1306_I2C(128, 32, i2c1)
oled1.fill(0)
oled1.show()

r_s = 0.1  # shunt resistor on INA219 board

# random variables
show = True
count = 0
while True:
    pin.toggle()    
    sleep(0.2)    
        
    # read INA219
    # current is returned in milliamps
    col_vsh_current = col_vsh.current
    col_vsh_bus_voltage = col_vsh.bus_voltage
    sys_5v_current = sys_5v.current
    sys_5v_bus_voltage = sys_5v.bus_voltage
    row_vgh_current = row_vgh.current
    row_vgh_bus_voltage = row_vgh.bus_voltage
    sys_3v3_current = sys_3v3.current
    sys_3v3_bus_voltage = sys_3v3.bus_voltage
    
    oled0.fill(0)
    # print("VSH : %3.1f/%3.1f" % (col_vsh_bus_voltage, col_vsh_current))
    oled0.text(("VSH : %3.1f/%3.1f" % (col_vsh_bus_voltage, col_vsh_current)), 0, 0)
    # print("VGH : %3.1f/%3.1f" % (row_vgh_bus_voltage, row_vgh_current))
    oled0.text(("VGH : %3.1f/%3.1f" % (row_vgh_bus_voltage, row_vgh_current)), 0, 8)
    # print("5V  : %3.1f/%3.1f" % (sys_5v_bus_voltage, sys_5v_current))
    oled0.text(("5V  : %3.1f/%3.1f" % (sys_5v_bus_voltage, sys_5v_current)), 0, 16)
    # print("3.3V: %3.1f/%3.1f" % (sys_3v3_bus_voltage, sys_3v3_current))
    oled0.text(("3.3V: %3.1f/%3.1f" % (sys_3v3_bus_voltage, sys_3v3_current)), 0, 24)
    oled0.show()    
    
    oled1.fill(0)
    oled1.text(("Fabric8 Labs"), 0, 0)
    oled1.text(("Panel Monitor"), 0, 8)
    oled1.show()
    
    # write out uart
    txData = b'%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f\n\r' % (col_vsh_bus_voltage, col_vsh_current, row_vgh_bus_voltage, row_vgh_current, sys_5v_bus_voltage, sys_5v_current, sys_3v3_bus_voltage, sys_3v3_current)
    print("%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f\n\r" % (col_vsh_bus_voltage, col_vsh_current, row_vgh_bus_voltage, row_vgh_current, sys_5v_bus_voltage, sys_5v_current, sys_3v3_bus_voltage, sys_3v3_current))
    uart0.write(txData)
