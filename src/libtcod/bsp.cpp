/* BSD 3-Clause License
 *
 * Copyright © 2008-2025, Jice and the libtcod contributors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "bsp.hpp"

#include <vector>

TCODBsp::TCODBsp(TCODBsp* father, bool left) {
  if (father->horizontal) {
    x = father->x;
    w = father->w;
    y = left ? father->y : father->position;
    h = left ? father->position - y : father->y + father->h - father->position;
  } else {
    y = father->y;
    h = father->h;
    x = left ? father->x : father->position;
    w = left ? father->position - x : father->x + father->w - father->position;
  }
  level = father->level + 1;
}

TCODBsp::~TCODBsp() { removeSons(); }

bool TCODBsp::traversePreOrder(ITCODBspCallback* listener, void* userData) {
  if (!listener->visitNode(this, userData)) return false;
  if (getLeft() && !getLeft()->traversePreOrder(listener, userData)) return false;
  if (getRight() && !getRight()->traversePreOrder(listener, userData)) return false;
  return true;
}

bool TCODBsp::traverseInOrder(ITCODBspCallback* listener, void* userData) {
  if (getLeft() && !getLeft()->traverseInOrder(listener, userData)) return false;
  if (!listener->visitNode(this, userData)) return false;
  if (getRight() && !getRight()->traverseInOrder(listener, userData)) return false;
  return true;
}

bool TCODBsp::traversePostOrder(ITCODBspCallback* listener, void* userData) {
  if (getLeft() && !getLeft()->traversePostOrder(listener, userData)) return false;
  if (getRight() && !getRight()->traversePostOrder(listener, userData)) return false;
  if (!listener->visitNode(this, userData)) return false;
  return true;
}

bool TCODBsp::traverseLevelOrder(ITCODBspCallback* listener, void* userData) {
  std::vector<TCODBsp*> stack;
  stack.push_back(this);
  while (!stack.empty()) {
    TCODBsp* node = stack.front();
    stack.erase(stack.begin());
    if (node->getLeft()) stack.push_back(node->getLeft());
    if (node->getRight()) stack.push_back(node->getRight());
    if (!listener->visitNode(node, userData)) return false;
  }
  return true;
}

bool TCODBsp::traverseInvertedLevelOrder(ITCODBspCallback* listener, void* userData) {
  std::vector<TCODBsp*> stack1;
  std::vector<TCODBsp*> stack2;
  stack1.push_back(this);
  while (!stack1.empty()) {
    TCODBsp* node = stack1.front();
    stack1.erase(stack1.begin());
    stack2.push_back(node);
    if (node->getLeft()) stack1.push_back(node->getLeft());
    if (node->getRight()) stack1.push_back(node->getRight());
  }
  while (!stack2.empty()) {
    if (!listener->visitNode(stack2.back(), userData)) return false;
    stack2.pop_back();
  }
  return true;
}

void TCODBsp::removeSons() {
  TCODBsp* node = static_cast<TCODBsp*>(sons);
  while (node) {
    TCODBsp* nextNode = static_cast<TCODBsp*>(node->next);
    node->removeSons();
    delete node;
    node = nextNode;
  }
  sons = NULL;
}

void TCODBsp::splitOnce(bool horizontal_, int position_) {
  this->horizontal = horizontal_;
  this->position = position_;
  addSon(new TCODBsp(this, true));
  addSon(new TCODBsp(this, false));
}

void TCODBsp::splitRecursive(
    TCODRandom* randomizer, int nb, int minHSize, int minVSize, float maxHRatio, float maxVRatio) {
  if (nb == 0 || (w < 2 * minHSize && h < 2 * minVSize)) return;
  bool horiz;
  if (!randomizer) randomizer = TCODRandom::getInstance();
  // promote square rooms
  if (h < 2 * minVSize || w > h * maxHRatio)
    horiz = false;
  else if (w < 2 * minHSize || h > w * maxVRatio)
    horiz = true;
  else
    horiz = randomizer->getInt(0, 1) == 0;
  int split_position;
  if (horiz) {
    split_position = randomizer->getInt(y + minVSize, y + h - minVSize);
  } else {
    split_position = randomizer->getInt(x + minHSize, x + w - minHSize);
  }
  splitOnce(horiz, split_position);
  getLeft()->splitRecursive(randomizer, nb - 1, minHSize, minVSize, maxHRatio, maxVRatio);
  getRight()->splitRecursive(randomizer, nb - 1, minHSize, minVSize, maxHRatio, maxVRatio);
}

void TCODBsp::resize(int x_, int y_, int w_, int h_) {
  this->x = x_;
  this->y = y_;
  this->w = w_;
  this->h = h_;
  if (getLeft()) {
    if (horizontal) {
      getLeft()->resize(x, y, w, position - y);
      getRight()->resize(x, position, w, y + h - position);
    } else {
      getLeft()->resize(x, y, position - x, h);
      getRight()->resize(position, y, x + w - position, h);
    }
  }
}

bool TCODBsp::contains(int px, int py) const { return (px >= x && py >= y && px < x + w && py < y + h); }

TCODBsp* TCODBsp::findNode(int px, int py) {
  if (!contains(px, py)) return NULL;
  if (!isLeaf()) {
    TCODBsp *left, *right;
    left = getLeft();
    if (left->contains(px, py)) return left->findNode(px, py);
    right = getRight();
    if (right->contains(px, py)) return right->findNode(px, py);
  }
  return this;
}
