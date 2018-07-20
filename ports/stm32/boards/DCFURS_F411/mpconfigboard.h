#define MICROPY_HW_BOARD_NAME       "DCFURS"
#define MICROPY_HW_MCU_NAME         "STM32F411xE"

#define MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE (0)
#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_HAS_MMA7660      (1)
#define MICROPY_HW_ENABLE_USB       (1)

// use external SPI flash for storage
#define MICROPY_HW_SPIFLASH_SIZE_BITS (32 * 1024 * 1024)
#define MICROPY_HW_SPI3_NSS         (pin_A15)
#define MICROPY_HW_SPI3_SCK         (pin_C10)
#define MICROPY_HW_SPI3_MOSI        (pin_C12)
#define MICROPY_HW_SPI3_MISO        (pin_C11)

// block device config for SPI flash
extern const struct _mp_spiflash_config_t spiflash_config;
extern struct _spi_bdev_t spi_bdev;
#define MICROPY_HW_BDEV_IOCTL(op, arg) ( \
    (op) == BDEV_IOCTL_NUM_BLOCKS ? (MICROPY_HW_SPIFLASH_SIZE_BITS / 8 / FLASH_BLOCK_SIZE) : \
    (op) == BDEV_IOCTL_INIT ? spi_bdev_ioctl(&spi_bdev, (op), (uint32_t)&spiflash_config) : \
    spi_bdev_ioctl(&spi_bdev, (op), (arg)) \
)
#define MICROPY_HW_BDEV_READBLOCKS(dest, bl, n) spi_bdev_readblocks(&spi_bdev, (dest), (bl), (n))
#define MICROPY_HW_BDEV_WRITEBLOCKS(src, bl, n) spi_bdev_writeblocks(&spi_bdev, (src), (bl), (n))

// HSE is 12MHz, CPU freq set to 84MHz
#define MICROPY_HW_CLK_PLLM (12)
#define MICROPY_HW_CLK_PLLN (336)
#define MICROPY_HW_CLK_PLLP (RCC_PLLP_DIV4)
#define MICROPY_HW_CLK_PLLQ (7)

// UART config
//#define MICROPY_HW_UART1_NAME   "X"
#define MICROPY_HW_UART1_TX     (pin_B6)
#define MICROPY_HW_UART1_RX     (pin_B7)

// I2C busses
//#define MICROPY_HW_I2C1_NAME "Y"
#define MICROPY_HW_I2C1_SCL (pin_B8)
#define MICROPY_HW_I2C1_SDA (pin_B9)

// USRSW has no pullup or pulldown. Pressing the button makes the input go high.
#define MICROPY_HW_USRSW_PIN        (pin_C15)
#define MICROPY_HW_USRSW_PULL       (GPIO_PULLDOWN)
#define MICROPY_HW_USRSW_EXTI_MODE  (GPIO_MODE_IT_RISING)
#define MICROPY_HW_USRSW_PRESSED    (1)

// Make pretend that we have an LED
#define MICROPY_HW_LED1             (pin_A13) // red
#define MICROPY_HW_LED2             (pin_A14) // green
#define MICROPY_HW_LED3             (pin_B3)  // blue
#define MICROPY_HW_LED_ON(pin)      (mp_hal_pin_low(pin))
#define MICROPY_HW_LED_OFF(pin)     (mp_hal_pin_high(pin))

// USB config
#define MICROPY_HW_USB_FS              (1)
#define MICROPY_HW_USB_VBUS_DETECT_PIN (pin_A9)
