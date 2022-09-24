/**
 * @file
 * @date 2022/9/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace lstg::Subsystem::Audio::detail
{
    /**
     * 静态分配的拓扑排序工具
     */
    template <size_t VertexCount>
    class StaticTopologicalSorter
    {
    public:
        StaticTopologicalSorter()
        {
            Reset();
        }

    public:
        /**
         * 设置邻接关系
         * @param vertexBegin 开始点
         * @param vertexEnd 终止点
         */
        void SetAdjacency(size_t vertexBegin, size_t vertexEnd) noexcept
        {
            assert(vertexBegin < VertexCount);
            assert(vertexEnd < VertexCount);
            m_stAdjacencyMatrix[vertexBegin][vertexEnd] = true;
        }

        /**
         * 清除邻接状态
         */
        void Reset() noexcept
        {
            ::memset(m_stAdjacencyMatrix, 0, sizeof(m_stAdjacencyMatrix));
        }

        /**
         * 执行拓扑排序
         * @return 当出现循环时，返回 false，否则返回 true
         */
        bool Sort() noexcept
        {
            ::memset(m_bVisited, 0, sizeof(m_bVisited));
            m_uResultAt = 0;
            ::memset(m_uResults, 0, sizeof(m_uResults));

            // 对每个顶点进行处理
            for (size_t i = 0; i < VertexCount; ++i)
            {
                auto state = m_bVisited[i];
                assert(state != STATE_VISITING);
                if (state == STATE_NOT_VISIT)
                {
                    auto ret = SortHelper(i);
                    if (!ret)
                        return false;
                }
            }
            return true;
        }

        /**
         * 获取结果
         * @param i 第几个顶点，0表示第一个被访问的节点
         * @return 节点ID
         */
        size_t GetResult(size_t i) noexcept
        {
            assert(i < VertexCount);
            return m_uResults[VertexCount - (i + 1)];
        }

    private:
        bool SortHelper(size_t vertexId) noexcept
        {
            assert(vertexId < VertexCount);
            assert(m_bVisited[vertexId] == STATE_NOT_VISIT);

            m_bVisited[vertexId] = STATE_VISITING;

            for (size_t i = 0; i < VertexCount; ++i)
            {
                // 从 vertexId 出发可以访问 i
                if (m_stAdjacencyMatrix[vertexId][i])
                {
                    auto visitState = m_bVisited[i];
                    if (visitState == STATE_VISITING)
                    {
                        return false;  // 出现环
                    }
                    else if (visitState == STATE_NOT_VISIT)
                    {
                        auto ret = SortHelper(i);
                        if (!ret)
                            return false;
                    }
                }
            }

            m_bVisited[vertexId] = STATE_VISITED;
            m_uResults[m_uResultAt++] = vertexId;
            return true;
        }

    private:
        enum {
            STATE_NOT_VISIT,
            STATE_VISITED,
            STATE_VISITING,
        };

        bool m_stAdjacencyMatrix[VertexCount][VertexCount];
        int8_t m_bVisited[VertexCount];
        size_t m_uResultAt = 0;
        size_t m_uResults[VertexCount];
    };
}
