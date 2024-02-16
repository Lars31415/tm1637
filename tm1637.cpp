/**
 * @file tm1637.cpp
 * @brief Implementation of the TM1637 class for controlling a 4-digit 7-segment display.
 */
#include "tm1637.hpp"

#include <pico/stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>
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
 * @brief Array of 7-segment LED segments for digits 0-9, a-z, space, dash, and star.
 */
// 0 - 9, a - z, blank, dash, star
uint8_t _SEGMENTS[] = {
    0x3F, // 	0	0
    0x06, // 	1	1
    0x5B, // 	2	2
    0x4F, // 	3	3
    0x66, // 	4	4
    0x6D, // 	5	5
    0x7D, // 	6	6
    0x07, // 	7	7
    0x7F, // 	8	8
    0x6F, // 	9	9
    0x77, // 	10	a
    0x7C, // 	11	b
    0x39, // 	12	c
    0x5E, // 	13	d
    0x79, // 	14	e
    0x71, // 	15	f
    0x3D, // 	16	g
    0x76, // 	17	h
    0x06, // 	18	i
    0x1E, // 	19	j
    0x76, // 	20	k
    0x38, // 	21	l
    0x55, // 	22	m
    0x54, // 	23	n
    0x5C, // 0x3F, // 	24	o
    0x73, // 	25	p
    0x67, // 	26	q
    0x50, // 	27	r
    0x6D, // 	28	s
    0x78, // 	29	t
    0x3E, // 	30	u
    0x1C, // 	31	v
    0x2A, // 	32	w
    0x76, // 	33	x
    0x6E, // 	34	y
    0x5B, // 	35	z
    0x00, // 	36	space
    0x40, // 	37	-
    0x63  //	38	*
};

/**
 * @brief Constructor for the TM1637 class.
 * @param clk Pin number for the clock (CLK) line.
 * @param dio Pin number for the data (DIO) line.
 * @param brightness Brightness level for the display (0-7).
 */
TM1637::TM1637(uint8_t clk, uint8_t dio, uint8_t brightness)
    : clk_(clk), dio_(dio), brightness_(std::min(uint8_t(0x07), brightness))
{
    gpio_init(clk_);
    gpio_set_dir(clk_, GPIO_OUT);
    gpio_pull_up(clk_);
    gpio_init(dio_);
    gpio_set_dir(dio_, GPIO_OUT);
    gpio_pull_up(dio_);
    gpio_put(clk_, 0);
    gpio_put(dio_, 0);

    _write_data_cmd();
    _write_dsp_ctrl();
}

/**
 * @brief Private method to start communication with the TM1637.
 */
void TM1637::_start()
{
    // std::cout << __FUNCTION__ << std::endl;
    gpio_put(clk_, 1);
    sleep_us(TM1637_DELAY);
    gpio_put(dio_, 1);
    sleep_us(TM1637_DELAY);
    gpio_put(dio_, 0);
    sleep_us(TM1637_DELAY);
    gpio_put(clk_, 0);
    sleep_us(TM1637_DELAY);
}

/**
 * @brief Private method to stop communication with the TM1637.
 */
void TM1637::_stop()
{
    // std::cout << __FUNCTION__ << std::endl;
    gpio_put(clk_, 0);
    sleep_us(TM1637_DELAY);
    gpio_put(dio_, 0);
    sleep_us(TM1637_DELAY);
    gpio_put(clk_, 1);
    sleep_us(TM1637_DELAY);
    gpio_put(dio_, 1);
}

/**
 * @brief Private method to send the data command to the TM1637.
 */
void TM1637::_write_data_cmd()
{
    // std::cout << __FUNCTION__ << std::endl;
    // automatic address increment, normal mode
    _start();
    _write_byte(TM1637_CMD1);
    _stop();
}

/**
 * @brief Private method to send the display control command to the TM1637.
 */
void TM1637::_write_dsp_ctrl()
{
    // std::cout << __FUNCTION__ << " " << (TM1637_CMD3 | TM1637_DSP_ON | brightness_) << std::endl;
    // display on, set brightness
    _start();
    _write_byte(TM1637_CMD3 | TM1637_DSP_ON | brightness_);
    _stop();
}

/**
 * @brief Private method to write a byte to the TM1637.
 * @param b The byte to be written.
 */
void TM1637::_write_byte(uint8_t b)
{
    // std::cout << __FUNCTION__ << " " << (uint)b << std::endl;
    for (int i = 0; i < 8; ++i)
    {
        gpio_put(dio_, (b >> i) & 1);
        sleep_us(TM1637_DELAY);
        gpio_put(clk_, 1);
        sleep_us(TM1637_DELAY);
        gpio_put(clk_, 0);
        sleep_us(TM1637_DELAY);
    }
    gpio_put(clk_, 0);
    sleep_us(TM1637_DELAY);
    gpio_put(clk_, 1);
    sleep_us(TM1637_DELAY);
    gpio_put(clk_, 0);
    sleep_us(TM1637_DELAY);
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
void TM1637::write(Segments segments, uint8_t pos)
{
    // Display up to 6 segments moving right from a given position.
    // The MSB in the 2nd segment controls the colon between the 2nd
    // and 3rd segments.
    pos = std::min(pos, uint8_t(0x05));
    _write_data_cmd();
    _start();

    _write_byte(TM1637_CMD2 | pos);

    // for seg in segments:
    // _write_byte(seg)
    for (size_t i = 0; i < segments.size(); ++i)
        _write_byte(segments.at(uint8_t(i / 3) * 6 + 2 - i));

    _stop();
    _write_dsp_ctrl();
}

/**
 * @brief Encode a decimal digit into a 7-segment LED segment.
 * @param digit The decimal digit to be encoded (0-9).
 * @return The encoded 7-segment LED segment.
 */
uint8_t TM1637::encode_digit(uint8_t digit)
{
    // Convert a character 0-9, a-f to a segment.
    return _SEGMENTS[digit & 0x0f];
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
    segments.resize(d);

    size_t j = 0;
    for (size_t i = 0; i < str.size(); ++i)
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
    while (segments.size() < 6)
        segments.push_back(encode_char(' '));
    return segments;
}

/**
 * @brief Encode a character into a 7-segment LED segment.
 * @param ch The input character.
 * @return The encoded 7-segment LED segment.
 */
uint8_t TM1637::encode_char(char ch)
{
    // Convert a character 0-9, a-z, space, dash or star to a segment."
    // o = ord(ch);
    if (ch == 32)
        return _SEGMENTS[36]; //  space
    if (ch == 42)
        return _SEGMENTS[38]; //  star/degrees
    if (ch == 45)
        return _SEGMENTS[37]; //  dash
    if ((ch >= 65) && (ch <= 90))
        return _SEGMENTS[ch - 55]; //  uppercase A-Z
    if ((ch >= 97) && (ch <= 122))
        return _SEGMENTS[ch - 87]; //  lowercase a-z
    if ((ch >= 48) && (ch <= 57))
        return _SEGMENTS[ch - 48]; //  0-9
    return _SEGMENTS[38];          //  star/degrees
}

/**
 * @brief Display a hexadecimal value on the TM1637 display.
 * @param val The hexadecimal value (0x0000 - 0xffff).
 */
void TM1637::hex(uint16_t val)
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