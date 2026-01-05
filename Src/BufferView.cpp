//
// Created by bohdan on 22.12.25.
//

#include "../Include/BufferView.h"
#include <cstring>

bool BufferStringView::operator==(const BufferStringView &Other) const
{
	return Size() == Other.Size() &&
		std::memcmp(Data(), Other.Data(), Size()) == 0;
}

bool operator==(const BufferStringView& Left, const std::string& Right)
{
	return Left.Size() == Right.size() &&
		   std::memcmp(Left.Data(), Right.data(), Left.Size()) == 0;
}

bool operator==(const std::string& Left, const BufferStringView& Right)
{
	return Right == Left;
}
