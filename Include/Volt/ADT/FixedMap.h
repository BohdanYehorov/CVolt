//
// Created by bohdan on 12.02.26.
//

#ifndef CVOLT_FIXEDMAP_H
#define CVOLT_FIXEDMAP_H

namespace Volt
{
	template <typename KeyT, typename ValueT>
	struct FixedMapBucket
	{
		KeyT Key;
		ValueT Value;

		FixedMapBucket() = default;
		FixedMapBucket(const KeyT& Key)
			: Key(Key) {}
		FixedMapBucket(const KeyT& Key, const ValueT& Value)
			: Key(Key), Value(Value) {}

		FixedMapBucket(const FixedMapBucket& Other)
			: Key(Other.Key), Value(Other.Value) {}
		FixedMapBucket(FixedMapBucket&& Other) noexcept
			: Key(std::move(Other.Key)), Value(std::move(Other.Value)) {}

		FixedMapBucket& operator=(const FixedMapBucket& Other)
		{
			if (this != &Other)
			{
				Key = Other.Key;
				Value = Other.Value;
			}
			return *this;
		}

		FixedMapBucket& operator=(FixedMapBucket&& Other) noexcept
		{
			if (this != &Other)
			{
				Key = std::move(Other.Key);
				Value = Other.Value;
			}
			return *this;
		}
	};

	template <typename KeyT, typename ValueT, typename Hasher = std::hash<KeyT>>
	class FixedMap
	{
	public:
		using SizeType = size_t;
		using BucketType = FixedMapBucket<KeyT, ValueT>;

	private:
		SizeType Len = 0;
		BucketType** Data = nullptr;

	public:
		FixedMap(SizeType Count);
		FixedMap(const std::initializer_list<BucketType>& List);
		~FixedMap();

		[[nodiscard]] const ValueT& operator[](const KeyT& Key) const;
		[[nodiscard]] ValueT& operator[](const KeyT& Key);

		[[nodiscard]] SizeType Length() const { return Len; }
		[[nodiscard]] BucketType* Find(const KeyT& Key);

	private:
		void Insert(const KeyT& Key, const ValueT& Value);
		ValueT& GetOrInsert(const KeyT& Key);
	};

	template<typename KeyT, typename ValueT, typename Hasher>
	FixedMap<KeyT, ValueT, Hasher>::FixedMap(SizeType Count)
		: Len(Count)
	{
		Data = new BucketType*[Len];

		for (size_t i = 0; i < Len; i++)
			Data[i] = nullptr;
	}

	template<typename KeyT, typename ValueT, typename Hasher>
	FixedMap<KeyT, ValueT, Hasher>::FixedMap(const std::initializer_list<BucketType> &List)
		: Len(List.size())
	{

		Data = new BucketType*[Len];

		for (size_t i = 0; i < Len; i++)
			Data[i] = nullptr;

		for (const auto& P : List)
			Insert(P.Key, P.Value);
	}

	template<typename KeyT, typename ValueT, typename Hasher>
	FixedMap<KeyT, ValueT, Hasher>::~FixedMap()
	{
		if (!Data) return;

		for (size_t i = 0; i < Len; i++)
			delete Data[i];

		delete[] Data;
	}

	template<typename KeyT, typename ValueT, typename Hasher>
	const ValueT & FixedMap<KeyT, ValueT, Hasher>::operator[](const KeyT &Key) const
	{
		SizeType Index = Hasher{}(Key) % Len;
		SizeType Start = Index;

		while (true)
		{
			if (Data[Index] && Data[Index]->Key == Key)
				return Data[Index]->Value;

			Index = (Index + 1) % Len;
			if (Index == Start)
				break;
		}

		throw std::runtime_error("Value by key not found");
	}

	template<typename KeyT, typename ValueT, typename Hasher>
	ValueT & FixedMap<KeyT, ValueT, Hasher>::operator[](const KeyT &Key)
	{
		return GetOrInsert(Key);
	}

	template<typename KeyT, typename ValueT, typename Hasher>
	FixedMap<KeyT, ValueT, Hasher>::BucketType * FixedMap<KeyT, ValueT, Hasher>::Find(const KeyT &Key)
	{
		SizeType Index = Hasher{}(Key) % Len;
		SizeType Start = Index;

		while (true)
		{
			if (Data[Index] && Data[Index]->Key == Key)
				return Data[Index];

			Index = (Index + 1) % Len;
			if (Index == Start)
				return nullptr;
		}
	}

	template<typename KeyT, typename ValueT, typename Hasher>
	void FixedMap<KeyT, ValueT, Hasher>::Insert(const KeyT &Key, const ValueT &Value)
	{
		SizeType Index = Hasher{}(Key) % Len;
		SizeType Start = Index;

		while (true)
		{
			if (!Data[Index])
			{
				Data[Index] = new BucketType(Key, Value);
				break;
			}

			Index = (Index + 1) % Len;
			if (Index == Start)
				break;
		}
	}

	template<typename KeyT, typename ValueT, typename Hasher>
	ValueT & FixedMap<KeyT, ValueT, Hasher>::GetOrInsert(const KeyT &Key)
	{
		SizeType Index = Hasher{}(Key) % Len;
		SizeType Start = Index;

		while (true)
		{
			if (!Data[Index])
			{
				Data[Index] = new BucketType(Key);
				return Data[Index]->Value;
			}

			if (Data[Index]->Key == Key)
				return Data[Index]->Value;

			Index = (Index + 1) % Len;
			if (Index == Start)
				break;
		}

		throw std::runtime_error("Value by key not found, or FixedMap full");
	}
}

#endif //CVOLT_FIXEDMAP_H