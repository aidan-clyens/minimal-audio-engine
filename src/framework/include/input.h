#ifndef __INPUT_H__
#define __INPUT_H__

/** @enum eInputType
 *  @brief Types of implementations of the IInput interface
 */
enum class eInputType
{
  None,
  AudioDevice,
  AudioFile,
  MidiFile,
};

/** @interface IInput
 */
class IInput
{
public:
  IInput(const eInputType type): m_input_type(type) {}

  eInputType get_type() const { return m_input_type; }

private:
  eInputType m_input_type = eInputType::None;
};

#endif // __INPUT_H__