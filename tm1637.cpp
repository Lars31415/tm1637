/**
 * @file tm1637.cpp
 * @brief Implementation of the TM1637 class for controlling a 4-digit 7-segment display.
 */
#include "tm1637.hpp"
#include "led8x7seg_codes.h"
#include "tm1637.pio.h"

#include <iomanip>
#include <iostream>
#include <pico/stdlib.h>
#include <sstream>
#include <utility>

/**
 * @brief TM1637 command for sending data to the display.
 */
const uint8_t TM1637_CMD1 = 0x40;

/**
 * @brief TM1637 command for addressing a specific digit on the display.
 */
const uint8_t TM1637_CMD2 = 0xC0;

/**
 * @brief TM1637 command for controlling the display.
 */
const uint8_t TM1637_CMD3 = 0x80;

/**
 * @brief TM1637 display control command for turning on the display.
 */
const uint8_t TM1637_DSP_ON = 0x08;

/**
 * @brief Time delay in microseconds between clock (clk) and data (dio) pulses.
 */
const uint8_t TM1637_DELAY = 10;

/**
 * @brief Most significant bit (MSB) indicating the decimal point or colon on the display.
 */
const uint8_t TM1637_MSB = 0x80;

/**
 * @brief Constructor for the TM1637 class.
 * @param clk Pin number for the clock (CLK) line.
 * @param dio Pin number for the data (DIO) line.
 * @param brightness Brightness level for the display (0-7).
 */
TM1637::TM1637(PIO pio, uint sm, uint8_t clk, uint8_t dio, uint8_t brightness)
    : pio_(pio), sm_(sm), clk_(clk), dio_(dio), brightness_(std::min(uint8_t(0x07), brightness)), ack(2)
{
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << std::dec << __LINE__ << std::endl;
    std::cout << "clk " << (uint)clk << " dio " << (uint)dio << std::endl;

    pio_ = pio;
    uint offset = pio_add_program(pio_, &tm1637_program);
    tm1637_program_init(pio_, sm_, offset, dio_, clk_);

    // Enable PIO interrupt
    // irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq_handler);

    // Enable interrupt source inside PIO
    // pio_set_irq0_source_enabled(pio_, pis_interrupt0, true);

    pio_sm_set_enabled(pio_, sm_, true);

    _write_data_cmd();
    _write_dsp_ctrl();
}

/**
 * @brief Private method to send the data command to the TM1637.
 */
void TM1637::_write_data_cmd()
{
    // std::cout << __FUNCTION__ << std::endl;
    // automatic address increment, normal mode
    uint32_t b = 0x000001 | (TM1637_CMD1 << 8);
    pio_sm_put_blocking(pio_, sm_, b);
}

/**
 * @brief Private method to send the display control command to the TM1637.
 */
void TM1637::_write_dsp_ctrl()
{
    // std::cout << __FUNCTION__ << " " << (TM1637_CMD3 | TM1637_DSP_ON | brightness_) << std::endl;
    // display on, set brightness
    uint32_t b = 0x000001 | ((TM1637_CMD3 | TM1637_DSP_ON | brightness_) << 8);
    pio_sm_put_blocking(pio_, sm_, b);
}

/**
 * @brief Set the brightness level of the display.
 * @param val Brightness level (0-7).
 * @return The updated brightness level.
 */
uint8_t TM1637::brightness(uint8_t val)
{
    // Set the display brightness 0-7."
    // brightness 0 = 1 / 16th pulse width
    // brightness 7 = 14 / 16th pulse width
    brightness_ = (val & 0x07);
    _write_data_cmd();
    _write_dsp_ctrl();
    return brightness_;
}

/**
 * @brief Write segments to the display starting from a specific position.
 * @param segments Array of 7-segment LED segments.
 * @param pos Starting position on the display (0-5).
 */
void TM1637::write(Segments segments)
{
    // Display up to 6 segments moving right from a given position.
    // The MSB in the 2nd segment controls the colon between the 2nd
    // and 3rd segments.
    // pos = std::min(pos, uint8_t(0x05));
    // _write_data_cmd();
    _write_data_cmd();
    uint32_t b = 7;
    b |= TM1637_CMD2 << 8;
    uint8_t byte = 2;
    for (size_t i = 0; i < 6; ++i)
    {
        uint32_t d = segments.at(uint8_t(i / 3) * 6 + 2 - i);
        b |= (d << 8 * byte);
        ++byte;
        if (byte == 4)
        {
            pio_sm_put_blocking(pio_, sm_, b);
            byte = 0;
            b = 0;
        }
    }
    if (byte > 0)
        pio_sm_put_blocking(pio_, sm_, b);
}

/**
 * @brief Encode a decimal digit into a 7-segment LED segment.
 * @param digit The decimal digit to be encoded (0-9).
 * @return The encoded 7-segment LED segment.
 */
uint8_t TM1637::encode_digit(uint8_t digit)
{
    // Convert a character 0-9, a-f to a segment.
    return hly_8dig_chars[digit]; // _SEGMENTS[digit & 0x0f];
}

/**
 * @brief Encode a decimal digit into a 7-segment LED segment.
 * @param digit The decimal digit to be encoded (0-9).
 * @return The encoded 7-segment LED segment.
 */
Segments TM1637::encode_string(std::string str)
{
    // Convert a string to LED segments.
    // Convert an up to 4 character length string containing 0-9, a-z,
    // space, dash, star and '.' to an array of segments, matching the length of
    // the source string.
    size_t d = 0;
    for (size_t i = 0; i < str.size(); ++i)
        if (str.at(i) != '.')
            ++d;
    Segments segments;

    size_t j = 0;
    for (size_t i = 0; (i < str.size() && (j < 6)); ++i)
    {
        if ((str.at(i) == '.') && (j > 0))
        {
            segments.at(j - 1) |= TM1637_MSB;
        }
        else
        {
            segments[j] = encode_char(str.at(i));
            j += 1;
        }
    }
    for (; j < 6; ++j)
        segments[j] = encode_char(' ');
    return segments;
}

/**
 * @brief Encode a character into a 7-segment LED segment.
 * @param ch The input character.
 * @return The encoded 7-segment LED segment.
 */
uint8_t TM1637::encode_char(char ch)
{
    return hly_8dig_chars[(uint8_t)ch];
}

/**
 * @brief Display a hexadecimal value on the TM1637 display.
 * @param val The hexadecimal value (0x0000 - 0xffff).
 */
void TM1637::hex(uint32_t val)
{
    // Display a hex value 0x0000 through 0xffff, right aligned."
    std::stringstream ss;
    ss << std::hex << std::setw(6) << val;
    write(encode_string(ss.str()));
}

/**
 * @brief Display a numeric value on the TM1637 display.
 * @param num The numeric value (-999 to 9999).
 */
void TM1637::number(uint32_t num)
{
    // Display a numeric value -999 through 9999, right aligned."
    // limit to range - 999 to 9999
    std::stringstream ss;
    ss << std::dec << std::setw(6) << num;
    write(encode_string(ss.str()));
}

/**
 * @brief Display a string on the TM1637 display.
 * @param str The input string.
 * @param colon Whether to display the colon symbol.
 */
void TM1637::show(std::string str, bool colon)
{
    Segments segments = encode_string(str);
    write(segments);
}

bool TM1637::is_present()
{
    return true;
}