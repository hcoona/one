// Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
// All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.

#include <cassert>
#include <initializer_list>
#include <ostream>

#include "gtest/gtest.h"

namespace hcoona {
namespace leetcode {
namespace {

/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode() : val(0), next(nullptr) {}
 *     ListNode(int x) : val(x), next(nullptr) {}
 *     ListNode(int x, ListNode *next) : val(x), next(next) {}
 * };
 */

struct ListNode {
  ListNode() = default;
  explicit ListNode(int x) : val(x) {}
  ListNode(int x, ListNode* next) : val(x), next(next) {}

  int val{};
  ListNode* next{};
};

std::ostream& operator<<(std::ostream& os, const ListNode& n) {
  return os << n.val;
}

struct ListNodeStreamPrintWrapper {
  explicit ListNodeStreamPrintWrapper(ListNode* head) : head(head) {}
  ListNode* head;
};

std::ostream& operator<<(std::ostream& os,
                         const ListNodeStreamPrintWrapper& wrapper) {
  ListNode* head = wrapper.head;
  os << "[" << *head;
  head = head->next;
  while (head != nullptr) {
    os << "," << *head;
    head = head->next;
  }
  return os << "]";
}

template <typename Container>
bool EqualTo(ListNode* head, const Container& c) {
  for (const auto& v : c) {
    if (head == nullptr || head->val != v) {
      return false;
    }
    head = head->next;
  }
  return head == nullptr;
}

template <typename T>
void IgnoreResult(T&& /*unused*/) {}

class Solution {
 public:
  ListNode* reverseKGroup(ListNode* head, int k);

 private:
  static int reverse_k(ListNode* group_pre_head, int k);
};

ListNode* Solution::reverseKGroup(ListNode* head, int k) {
  if (head == nullptr || k == 0 || k == 1) {
    return head;
  }

  ListNode global_pre_head;
  global_pre_head.next = head;

  ListNode* group_pre_head = &global_pre_head;
  while (true) {
    ListNode* group_last = group_pre_head->next;
    if (group_last == nullptr) {
      // It's an empty group, no need to process further.
      break;
    }

    int count = reverse_k(group_pre_head, k);
    if (count != k) {
      IgnoreResult(reverse_k(group_pre_head, k));
      break;
    }

    // Move to process next group.
    group_pre_head = group_last;
  }

  return global_pre_head.next;
}

// static
int Solution::reverse_k(ListNode* group_pre_head, int k) {
  assert(group_pre_head != nullptr);
  assert(group_pre_head->next != nullptr);

  int origin_k = k;
  ListNode* group_tail = group_pre_head->next;

  ListNode* previous = group_pre_head;
  ListNode* current = previous->next;
  while ((k--) != 0) {
    if (current == nullptr) {
      break;
    }
    ListNode* next = current->next;

    current->next = previous;
    previous = current;
    current = next;
  }
  group_pre_head->next = previous;
  group_tail->next = current;
  return origin_k - (k + 1);
}

}  // namespace

TEST(Test25, Case01) {
  ListNode nodes[5];
  nodes[0].val = 1;
  nodes[1].val = 2;
  nodes[2].val = 3;
  nodes[3].val = 4;
  nodes[4].val = 5;
  nodes[0].next = &nodes[1];
  nodes[1].next = &nodes[2];
  nodes[2].next = &nodes[3];
  nodes[3].next = &nodes[4];
  nodes[4].next = nullptr;

  Solution s;
  ListNode* head = s.reverseKGroup(&nodes[0], 2);
  EXPECT_TRUE(EqualTo<std::initializer_list<int>>(head, {2, 1, 4, 3, 5}))
      << ListNodeStreamPrintWrapper(head);
}

TEST(Test25, Case02) {
  ListNode nodes[5];
  nodes[0].val = 1;
  nodes[1].val = 2;
  nodes[2].val = 3;
  nodes[3].val = 4;
  nodes[4].val = 5;
  nodes[0].next = &nodes[1];
  nodes[1].next = &nodes[2];
  nodes[2].next = &nodes[3];
  nodes[3].next = &nodes[4];
  nodes[4].next = nullptr;

  Solution s;
  ListNode* head = s.reverseKGroup(&nodes[0], 3);
  EXPECT_TRUE(EqualTo<std::initializer_list<int>>(head, {3, 2, 1, 4, 5}))
      << ListNodeStreamPrintWrapper(head);
}

TEST(Test25, Case03) {
  ListNode nodes[5];
  nodes[0].val = 1;
  nodes[1].val = 2;
  nodes[2].val = 3;
  nodes[3].val = 4;
  nodes[4].val = 5;
  nodes[0].next = &nodes[1];
  nodes[1].next = &nodes[2];
  nodes[2].next = &nodes[3];
  nodes[3].next = &nodes[4];
  nodes[4].next = nullptr;

  Solution s;
  ListNode* head = s.reverseKGroup(&nodes[0], 5);
  EXPECT_TRUE(EqualTo<std::initializer_list<int>>(head, {5, 4, 3, 2, 1}))
      << ListNodeStreamPrintWrapper(head);
}

}  // namespace leetcode
}  // namespace hcoona
