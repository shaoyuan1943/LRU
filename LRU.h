#pragma once
#ifndef _LRU_H_
#define _LRU_H_

#include <string>
#include <functional>
#include <map>
#include <list>

namespace lru
{
	template<typename T>
	class Entry final
	{
	public:
		std::wstring key;
		T value;
	};

	template<typename T>
	using Iter = typename std::list<Entry<T>>::iterator;

	template<typename T>
	using EvictCallback = typename std::function<void(const Entry<T>& t)>;

	template<typename T>
	class List final
	{
	public:
		List()
		{
		}

		~List()
		{
			lst_.clear();
		}

		unsigned int Capacity() const
		{
			return capacity_;
		}

		Iter<T> PushFront(const Entry<T>& entry)
		{
			++capacity_;
			return lst_.insert(lst_.begin(), entry);
		}

		Iter<T> MoveToFront(const Iter<T>& it)
		{
			auto v = (*it);
			lst_.erase(it);
			return lst_.insert(lst_.begin(), v);
		}

		bool Remove(const Iter<T>& it)
		{
			if (lst_.size() > 0 && capacity_ > 0)
			{
				--capacity_;
				lst_.erase(it);
				return true;
			}
			
			return false;
		}
		
		Iter<T> Back()
		{
			if (lst_.size() > 0 && capacity_ > 0)
			{
				return (--lst_.end());
			}

			return lst_.end();
		}
		bool IsEnd(Iter<T> it) const
		{
			return it != lst_.end();
		}
	private:
		unsigned int capacity_{ 0 };
		std::list<Entry<T>> lst_;
	};

	template<typename T>
	class LRU final
	{
	public:
		LRU(const unsigned int& fixed, const EvictCallback<T>& callback = nullptr)
		{
			if (fixedCapacity_ != fixed)
				fixedCapacity_ = fixed;

			if (callback != nullptr)
				evictCallback_ = callback;
		}

		~LRU()
		{}

		bool Put(const std::wstring& key, const T& value)
		{
			if (key.empty())
				return false;

			auto it = lruMap_.find(key);
			if (it != lruMap_.end())
			{
				auto newIt = lruList_.MoveToFront(it->second);
				lruMap_[key] = newIt;
				return false;
			}

			if (fixedCapacity_ <= lruList_.Capacity())
			{
				auto delIt = lruList_.Back();
				if (lruList_.IsEnd(delIt))
				{
					_Delete(delIt);
				}
				else
					return false;
			}

			Entry<T> entry;
			entry.key = key;
			entry.value = value;
			auto iter = lruList_.PushFront(entry);
			lruMap_[key] = iter;
			return true;
		}

		T* Get(const std::wstring& key)
		{
			if (key.empty())
				return nullptr;

			auto it = lruMap_.find(key);
			if (it == lruMap_.end())
				return nullptr;

			auto newIt = lruList_.MoveToFront(it->second);
			lruMap_[key] = newIt;
			return &newIt->value;
		}

		bool Del(const std::wstring& key)
		{
			if (key.empty())
				return false;

			if (lruList_.Capacity() <= 0)
				return false;

			auto it = lruMap_.find(key);
			auto entry = (*it);
			if (it == lruMap_.end())
				return false;

			_Delete(it->second);
			return true;
		}
	
	private:
		void _Delete(const Iter<T>& it)
		{
			if (!lruList_.IsEnd(it))
				return;

			auto entry = *(it);
			if (lruList_.Remove(it))
			{
				if (evictCallback_ != nullptr)
					evictCallback_(entry);

				lruMap_.erase(entry.key);
			}
		}

	private:
		unsigned int fixedCapacity_{ 100 };
		std::map<const std::wstring, Iter<T>> lruMap_;
		List<T> lruList_;
		EvictCallback<T> evictCallback_{ nullptr };
	};
}
#endif
