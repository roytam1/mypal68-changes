/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef frontend_DestructuringFlavor_h
#define frontend_DestructuringFlavor_h

namespace js {
namespace frontend {

enum class DestructuringFlavor {
  // Destructuring into a declaration.
  Declaration,

  // Destructuring into a formal parameter, when the formal parameters
  // contain an expression that might be evaluated, and thus require
  // this destructuring to assign not into the innermost scope that
  // contains the function body's vars, but into its enclosing scope for
  // parameter expressions.
  FormalParameterInVarScope,

  // Destructuring as part of an AssignmentExpression.
  Assignment
};

} /* namespace frontend */
} /* namespace js */

#endif /* frontend_DestructuringFlavor_h */
