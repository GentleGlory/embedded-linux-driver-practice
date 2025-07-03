#ifndef _LED_TEST_H
#define _LED_TEST_H

#define GPIO_GPFSEL_ADDR(gpio_base, pin_num)	( ((unsigned char*)(gpio_base)) + ( (pin_num) / 10) * 4)
#define GPIO_GPPUD_ADDR(gpio_base)		( ((unsigned char*)(gpio_base)) + 0x94)
#define GPIO_GPPUDCLK_ADDR(gpio_base, pin_num)	( ((unsigned char*)(gpio_base)) + 0x98 + ( (pin_num) / 32) * 4)

#define GPIO_GPSET_ADDR(gpio_base, pin_num)    ( ((unsigned char*)(gpio_base)) + 0x1C + ( (pin_num) / 32) * 4)
#define GPIO_GPCLR_ADDR(gpio_base, pin_num)    ( ((unsigned char*)(gpio_base)) + 0x28 + ( (pin_num) / 32) * 4)

#define GPIO_GP_SET_CLR_ASSERT_LINE(pin_num)	( 1 << ( (pin_num) % 32) )

#define GPIO_FUNCTION_SELECT_MASK(pin_num) 	(~(0x7 << (( (pin_num) % 10) * 3 )))
#define GPIO_FUNCTION_SELECT_INPUT(pin_num) 	(0x0 << (( (pin_num) % 10) * 3 ))
#define GPIO_FUNCTION_SELECT_OUTPUT(pin_num) 	(0x1 << (( (pin_num) % 10) * 3 ))
#define GPIO_FUNCTION_SELECT_FUNC_0(pin_num) 	(0x4 << (( (pin_num) % 10) * 3 ))
#define GPIO_FUNCTION_SELECT_FUNC_1(pin_num) 	(0x5 << (( (pin_num) % 10) * 3 ))
#define GPIO_FUNCTION_SELECT_FUNC_2(pin_num) 	(0x6 << (( (pin_num) % 10) * 3 ))
#define GPIO_FUNCTION_SELECT_FUNC_3(pin_num) 	(0x7 << (( (pin_num) % 10) * 3 ))
#define GPIO_FUNCTION_SELECT_FUNC_4(pin_num) 	(0x3 << (( (pin_num) % 10) * 3 ))
#define GPIO_FUNCTION_SELECT_FUNC_5(pin_num) 	(0x2 << (( (pin_num) % 10) * 3 ))

#define GPIO_GPPUDCLK_ASSERT_LINE(pin_num)	( 1 << ( (pin_num) % 32) )  





#endif