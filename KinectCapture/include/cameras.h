/* to modify original OpenKinect Project. http://www.openkinect.org 
 @ https://github.com/OpenKinect/libfreenect
 @
 @ The following functions are to be redefined and added with the corresponding definitions differing from
** the original ones only in the modifiers and names to use outside library */

#define write_register_mine write_register
#define read_cmos_register_mine read_cmos_register

FREENECTAPI uint16_t read_register_mine(freenect_device *dev, uint16_t reg);
FREENECTAPI int write_register_mine(freenect_device *dev, uint16_t reg, uint16_t data);
FREENECTAPI uint16_t read_cmos_register_mine(freenect_device *dev, uint16_t reg);
FREENECTAPI int write_cmos_register_mine(freenect_device *dev, uint16_t reg, uint16_t value);



