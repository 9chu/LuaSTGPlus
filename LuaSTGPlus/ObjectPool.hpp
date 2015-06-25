#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief ���������
	template <typename T, size_t AllocCount>
	class FixedObjectPool
	{
	private:
		static_assert(std::is_pod<typename T>::value, "T must be a pod type.");

		std::vector<size_t> m_FreeIndex;  // ���пռ�������
		std::array<bool, AllocCount> m_DataUsed;  // ���ÿռ���
		std::array<T, AllocCount> m_DataBuffer;  // ���пռ�
	public:
		/// @brief ����һ������
		/// @param[out] id ����id
		/// @return �Ƿ�ɹ���ʧ�ܷ���false�������������
		bool Alloc(size_t& id)
		{
			if (m_FreeIndex.size() > 0)
			{
				id = m_FreeIndex.back();
				m_FreeIndex.pop_back();
				m_DataUsed[id] = true;
				return true;
			}
			id = static_cast<size_t>(-1);
			return false;
		}
		/// @brief ����һ������
		void Free(size_t id)
		{
			if (id < AllocCount && m_DataUsed[id])
			{
				m_DataUsed[id] = false;
				m_FreeIndex.push_back(id);
			}
		}
		/// @brief ��ȡ���������
		/// @return ��id��Ч����nullptr
		T* Data(size_t id)
		{
			if (id < AllocCount && m_DataUsed[id])
				return &m_DataBuffer[id];
			return nullptr;
		}
		/// @brief �����ѷ��������
		size_t Size()
		{
			return m_DataBuffer.size() - m_FreeIndex.size();
		}
		/// @brief ��ն���ز��������ж���
		void Clear()
		{
			m_FreeIndex.resize(m_DataBuffer.size());
			for (size_t i = 0; i < m_DataBuffer.size(); ++i)
			{
				m_FreeIndex[i] = (m_DataBuffer.size() - 1) - i;
				m_DataUsed[i] = false;
			}
		}
	private:
		FixedObjectPool& operator=(const FixedObjectPool&);
		FixedObjectPool(const FixedObjectPool&);
	public:
		FixedObjectPool()
		{
			Clear();
		}
	};
}
