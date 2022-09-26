/**
 * @file
 * @author 9chu
 * @date 2022/9/26
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cassert>

namespace lstg
{
    /**
     * 侵入式链表节点
     */
    struct IntrusiveListNode
    {
        IntrusiveListNode* Prev = nullptr;
        IntrusiveListNode* Next = nullptr;

        IntrusiveListNode() noexcept = default;
        
        IntrusiveListNode(IntrusiveListNode&& org) noexcept
            : Prev(org.Prev), Next(org.Next)
        {
            if (Prev)
                Prev->Next = this;
            if (Next)
                Next->Prev = this;
            org.Prev = org.Next = nullptr;
        }
    };
    
    /**
     * 在指定节点前插入节点
     * @param dest 目的节点
     * @param self 要插入的节点
     */
    inline void ListInsertBefore(IntrusiveListNode* dest, IntrusiveListNode* self) noexcept
    {
        assert(dest);
        assert(self && !self->Prev && !self->Next);
        self->Prev = dest->Prev;
        self->Next = dest;
        
        if (dest->Prev)
            dest->Prev->Next = self;
        dest->Prev = self;
    }

    /**
     * 在指定节点后插入节点
     * @param dest 目的节点
     * @param self 要插入的节点
     */
    inline void ListInsertAfter(IntrusiveListNode* dest, IntrusiveListNode* self) noexcept
    {
        assert(dest);
        assert(self && !self->Prev && !self->Next);
        self->Prev = dest;
        self->Next = dest->Next;

        if (dest->Next)
            dest->Next->Prev = self;
        dest->Next = self;
    }

    /**
     * 将节点从链表移除
     * @param self 节点
     */
    inline void ListRemove(IntrusiveListNode* self) noexcept
    {
        if (self->Prev)
            self->Prev->Next = self->Next;
        if (self->Next)
            self->Next->Prev = self->Prev;
        self->Next = nullptr;
        self->Prev = nullptr;
    }

    /**
     * 在有序链表上对节点进行插入排序
     * @tparam TComparer
     * @param self
     * @param comparer
     */
    template <typename TComparer>
    inline void ListInsertSort(IntrusiveListNode* self, TComparer comparer) noexcept
    {
        // 插入排序
        // NOTE: 这里会保证头尾节点不参与排序过程
        assert(self->Next && self->Prev);
        if (self->Next->Next && comparer(self->Next, self))
        {
            // 向后插入
            auto insertBefore = self->Next->Next;
            while (insertBefore->Next && comparer(insertBefore, self))
                insertBefore = insertBefore->Next;
            ListRemove(self);
            ListInsertBefore(insertBefore, self);
        }
        else if (self->Prev->Prev && comparer(self, self->Prev))
        {
            // 向前插入
            auto insertAfter = self->Prev->Prev;
            while (insertAfter->Prev && comparer(self, insertAfter))
                insertAfter = insertAfter->Prev;
            ListRemove(self);
            ListInsertAfter(insertAfter, self);
        }
    }
}
