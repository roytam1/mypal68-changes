/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsChildContentList_h__
#define nsChildContentList_h__

#include "nsISupportsImpl.h"
#include "nsINodeList.h"   // base class
#include "js/TypeDecls.h"  // for Handle, Value, JSObject, JSContext

class nsIContent;
class nsINode;

/**
 * Class that implements the nsINodeList interface (a list of children of
 * the content), by holding a reference to the content and delegating Length
 * and Item to its existing child list.
 * @see nsINodeList
 */
class nsAttrChildContentList : public nsINodeList {
 public:
  explicit nsAttrChildContentList(nsINode* aNode) : mNode(aNode) {}

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SKIPPABLE_SCRIPT_HOLDER_CLASS(nsAttrChildContentList)

  // nsWrapperCache
  virtual JSObject* WrapObject(JSContext* cx,
                               JS::Handle<JSObject*> aGivenProto) override;

  // nsINodeList interface
  virtual int32_t IndexOf(nsIContent* aContent) override;
  virtual nsIContent* Item(uint32_t aIndex) override;
  uint32_t Length() override;

  virtual void DropReference() { mNode = nullptr; }

  virtual nsINode* GetParentObject() override { return mNode; }

 protected:
  virtual ~nsAttrChildContentList() {}

 private:
  // The node whose children make up the list.
  // This is a non-owning ref which is safe because it's set to nullptr by
  // DropReference() by the node slots get destroyed.
  nsINode* MOZ_NON_OWNING_REF mNode;
};

class nsParentNodeChildContentList final : public nsAttrChildContentList {
 public:
  explicit nsParentNodeChildContentList(nsINode* aNode)
      : nsAttrChildContentList(aNode), mIsCacheValid(false) {
    ValidateCache();
  }

  // nsINodeList interface
  virtual int32_t IndexOf(nsIContent* aContent) override;
  virtual nsIContent* Item(uint32_t aIndex) override;
  uint32_t Length() override;

  void DropReference() override {
    InvalidateCache();
    nsAttrChildContentList::DropReference();
  }

  void InvalidateCache() {
    mIsCacheValid = false;
    mCachedChildArray.Clear();
  }

 private:
  ~nsParentNodeChildContentList() {}

  // Return true if validation succeeds, false otherwise
  bool ValidateCache();

  // Whether cached array of child nodes is valid
  bool mIsCacheValid;

  // Cached array of child nodes
  AutoTArray<nsIContent*, 8> mCachedChildArray;
};

#endif /* nsChildContentList_h__ */
