#include "storage.h"
#include "spi.h"

const spi_bdev_obj_t bdev_spi_bus = {
    .spi = &spi_obj[2],
    .baudrate = 80000000,
    .polarity = 0,
    .phase = 0,
    .bits = 8,
    .firstbit = SPI_FIRSTBIT_MSB,
};

const mp_spiflash_config_t spiflash_config = {
    .bus_kind = MP_SPIFLASH_BUS_SPI,
    .bus.u_spi.cs = MICROPY_HW_SPI3_NSS,
    .bus.u_spi.data = (void*)&bdev_spi_bus,
    .bus.u_spi.proto = &spi_bdev_proto,
};

spi_bdev_t spi_bdev;
