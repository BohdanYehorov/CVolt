//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_DEBUGOUTPUT_H
#define CVOLT_DEBUGOUTPUT_H

#include  <ostream>

namespace Volt
{
	class DebugOutput
	{
		std::ostream& Os;

		DebugOutput(std::ostream& Os) : Os(Os) {}
	};
}

#endif //CVOLT_DEBUGOUTPUT_H