/**
 * @file my_tm1637.hpp
 * @brief Header file for the TM1637 class for controlling a 4-digit 7-segment display.
 */

#ifndef MY_TM1637_HPP
#define MY_TM1637_HPP

#include <array>
#include <cstdint>
#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <string>

/**
 * @typedef Segments
 * @brief Type definition for an array of 7-segment LED segments.
 */
typedef std::array<uint8_t, 6> Segments;

/**
 * @class TM1637
 * @brief Class for controlling a 4-digit 7-segment display using the TM1637 driver.
 */
class TM1637
{
  public:
    /**
     * @brief Constructor for the TM1637 class.
     * @param clk Pin number for the clock (CLK) line.
     * @param dio Pin number for the data (DIO) line.
     * @param brightness Brightness level for the display (0-7).
     */
    TM1637(PIO pio, uint sm, uint8_t clk, uint8_t dio, uint8_t brightness = 7);

    /**
     * @brief Set the brightness level of the display.
     * @param val Brightness level (0-7).
     * @return The updated brightness level.
     */
    uint8_t brightness(uint8_t val = 4);

    /**
     * @brief Write segments to the display starting from a specific position.
     * @param segments Array of 7-segment LED segments.
     * @param pos Starting position on the display (0-5).
     */
    void write(Segments segments);

    /**
     * @brief Encode a decimal digit into a 7-segment LED segment.
     * @param digit The decimal digit to be encoded (0-9).
     * @return The encoded 7-segment LED segment.
     */
    uint8_t encode_digit(uint8_t digit);

    /**
     * @brief Encode a string into an array of 7-segment LED segments.
     * @param str The input string.
     * @return Array of 7-segment LED segments.
     */
    Segments encode_string(std::string str);

    /**
     * @brief Encode a character into a 7-segment LED segment.
     * @param ch The input character.
     * @return The encoded 7-segment LED segment.
     */
    uint8_t encode_char(char ch);

    /**
     * @brief Display a hexadecimal value on the TM1637 display.
     * @param val The hexadecimal value (0x0000 - 0xffff).
     */
    void hex(uint32_t val);

    /**
     * @brief Display a numeric value on the TM1637 display.
     * @param num The numeric value (-999 to 9999).
     */
    void number(uint32_t num);

    /**
     * @brief Display a string on the TM1637 display.
     * @param str The input string.
     * @param colon Whether to display the colon symbol.
     */
    void show(std::string str, bool colon = false);

    bool is_present();

  private:
    PIO pio_;
    uint sm_;
    uint8_t clk_;        ///< Pin number for the clock (CLK) line.
    uint8_t dio_;        ///< Pin number for the data (DIO) line.
    uint8_t brightness_; ///< Brightness level for the display (0-7).
    int ack;
    Segments segments_;

    /**
     * @brief Private method to send the data command to the TM1637.
     */
    void _write_data_cmd();

    /**
     * @brief Private method to send the display control command to the TM1637.
     */
    void _write_dsp_ctrl();
};

#endif // MY_TM1637_HPP
