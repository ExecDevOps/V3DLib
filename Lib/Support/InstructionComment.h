#ifndef _LIB_COMMON_INSTRUCTIONCOMMENT_H
#define _LIB_COMMON_INSTRUCTIONCOMMENT_H
#include <string>
#include "basics.h"

namespace QPULib {

/**
 * Mixin for instruction comments
 */
class InstructionComment {
public:
	void header(std::string const &msg);
	void comment(std::string msg);
	std::string const &header() const { return m_header; }
	std::string const &comment() const { return m_comment; }

private:
	std::string m_header;
	std::string m_comment;
};

}  // namespace QPULib

#endif  // _LIB_COMMON_INSTRUCTIONCOMMENT_H
