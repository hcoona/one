#include <cstring>

#include "gtest/gtest.h"
#include "zkclient/zkadapter.h"
#include "zookeeper.h"  // NOLINT

static_assert(sizeof(hcoona::zookeeper::Id) == sizeof(Id),
              "sizeof(struct Id) mismatch!");
static_assert(sizeof(hcoona::zookeeper::Acl) == sizeof(ACL),
              "sizeof(struct ACL) mismatch!");

TEST(ZookeeperAdapterTest, IdAnyoneEquality) {
  const Id* p1 = reinterpret_cast<const Id*>(&hcoona::zookeeper::Id::kAnyone);
  const Id* p2 = &ZOO_ANYONE_ID_UNSAFE;
  EXPECT_EQ(std::string(p1->scheme), std::string(p2->scheme));
  EXPECT_EQ(std::string(p1->id), std::string(p2->id));
}

TEST(ZookeeperAdapterTest, AclOpenEquality) {
  const ACL* p1 =
      reinterpret_cast<const ACL*>(&hcoona::zookeeper::Acl::kOpen.at(0));
  const ACL* p2 = &(ZOO_OPEN_ACL_UNSAFE.data[0]);
  EXPECT_EQ(p1->perms, p2->perms);
  EXPECT_EQ(std::string(p1->id.scheme), std::string(p2->id.scheme));
  EXPECT_EQ(std::string(p1->id.id), std::string(p2->id.id));
}
