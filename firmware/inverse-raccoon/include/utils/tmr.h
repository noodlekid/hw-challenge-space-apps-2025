/**
 * @file tmr.h
 * @brief Triple Modular Redundancy template class
 */

#ifndef TMR_H
#define TMR_H

/**
 * @brief Triple Modular Redundancy for critical variables
 * @tparam T Type to protect with TMR
 */
template<typename T>
class TMR {
private:
  T value[3];

public:
  /**
   * @brief Default constructor
   */
  TMR() : value{0, 0, 0} {}
  
  /**
   * @brief Write value to all three copies
   * @param v Value to write
   */
  void write(T v) {
    value[0] = value[1] = value[2] = v;
  }
  
  /**
   * @brief Read value using 2-of-3 majority voting
   * @return Voted value
   */
  T vote() const {
    if (value[0] == value[1]) return value[0];
    if (value[0] == value[2]) return value[0];
    return value[1];
  }
  
  /**
   * @brief Validate that at least 2 of 3 copies match
   * @return true if valid, false if corrupted
   */
  bool validate() const {
    return (value[0] == value[1]) || 
           (value[1] == value[2]) || 
           (value[0] == value[2]);
  }
};

#endif // TMR_H}));
