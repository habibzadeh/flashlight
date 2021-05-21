/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <optional>

#include "flashlight/fl/tensor/Shape.h"
#include "flashlight/fl/tensor/Types.h"

namespace fl {

/**
 * Enum for various tensor backends.
 */
enum class TensorBackend { ArrayFire };

// Forward declaration - see TensorAdapter.h
class TensorAdapterBase;

/**
 * A Tensor on which computation can be performed.
 *
 * Underlying implementations of tensor operations are implemented via types
 * derived from `TensorAdapterBase`; these implementations also store state
 * associated with the tensor. Tensor stores a pointer to the implementation,
 * which can be swapped out if the implementation of backend
 * changes.`TensorAdapterBase` implementations may differ across tensor
 * libraries, hardware-specific libraries or compilers and DSLs.
 *
 * TODO:fl::Tensor {doc} more documentation. NOTE: this API will break
 * frequently and is not yet stable.
 */
class Tensor {
  // The backend adapter for the tensor
  std::unique_ptr<TensorAdapterBase> impl_;

 public:
  explicit Tensor(std::unique_ptr<TensorAdapterBase> adapter);
  Tensor() = default; // TODO: default construct an empty tensor with the
                      // default active backend
  virtual ~Tensor();

  /**
   * Get the shape of a tensor.
   *
   * @return the shape of the tensor
   */
  const Shape& shape() const;

  /**
   * Get the data type of tensor.
   *
   * @return the dtype of the tensor
   */
  dtype type() const;

  /**
   * Returns a tensor with elements cast as a particular type
   *
   * @param[in] the type to which to cast the tensor
   * @return a tensor with element-wise cast to the new type
   */
  Tensor astype(const dtype type);

  /**
   * Gets the backend enum from the underlying TensorAdapter.
   *
   * @return the backend in question
   */
  TensorBackend backend() const;

  /**
   * Gets the underlying tensor adapter implementation.
   *
   * @return the tensor adapter.
   */
  template <typename T>
  T& getAdapter() const {
    return *static_cast<T*>(impl_.get());
  }
};

/******************** Tensor Creation Functions ********************/
/**
 * Creates a new tensor with a given shape and filled with a particular value.
 *
 * @param[in] dims the shape of the tensor to create
 * @param[in] val the value with which to fill the tensor
 * @param[in] type the type of the tensor to create. Defaults to a value based
 * on the value type
 * @return a tensor of the specified shape filled with the specified value
 */
template <typename T>
Tensor full(
    const Shape& dims,
    const T& val,
    const dtype type = dtype_traits<T>::ctype);

/**
 * Return a the identity tensor of a given size and type.
 *
 * @param[in] dim the size of the dimension of the matrix (dim x dim)
 * @param[in] type the type of the resulting matrix
 */
Tensor identity(const Dim dim, const dtype type = dtype::f32);

/************************** Unary Operators ***************************/
/**
 * Element-wise negation of a tensor.
 *
 * @param[in] tensor the input tensor to negate.
 * @return a tensor with elements negated.
 */
Tensor negative(const Tensor& tensor);
inline Tensor operator-(const Tensor& tensor) {
  return negative(tensor);
}

/**
 * Performs element-wise logical-not on the elements of a tensor
 *
 * @param[in] tensor the tensor on which to perform logical not
 * @return a tensor with element-wise logical not of the input
 */
Tensor logicalNot(const Tensor& tensor);
inline Tensor operator!(const Tensor& tensor) {
  return logicalNot(tensor);
}

/**
 * Compute the element-wise exponential of a tensor
 *
 * @param[in] tensor the tensor to exponentiate
 * @return the exponentiated tensor
 */
Tensor exp(const Tensor& tensor);

/**
 * Compute the element-wise natural logarithm of a tensor
 *
 * @param[in] tensor the tensor on which to compute
 * @return the resulting tensor
 */
Tensor log(const Tensor& tensor);

/**
 * Returns the natural logarithm of one plus the input, element-wise.
 *
 * @param[in] tensor the tensor on which to compute
 * @return the resulting tensor
 */
Tensor log1p(const Tensor& tensor);

/**
 * Returns the element-wise sine of the input.
 *
 * @param[in] tensor the tensor on which to compute
 * @return the resulting tensor
 */
Tensor sin(const Tensor& tensor);

/**
 * Returns the element-wise cosine of the input.
 *
 * @param[in] tensor the tensor on which to compute
 * @return the resulting tensor
 */
Tensor cos(const Tensor& tensor);

/**
 * Returns the element-wise non-negative square root of the input.
 *
 * @param[in] tensor the tensor on which to compute
 * @return the resulting tensor
 */
Tensor sqrt(const Tensor& tensor);

/**
 * Returns the element-wise hyperbolic tangent of the input.
 *
 * @param[in] tensor the tensor on which to compute
 * @return the resulting tensor
 */
Tensor tanh(const Tensor& tensor);

/**
 * Returns the element-wise absolute value of the input.
 *
 * @param[in] tensor the tensor on which to compute
 * @return the resulting tensor
 */
Tensor absolute(const Tensor& tensor);
inline Tensor abs(const Tensor& tensor) {
  return absolute(tensor);
}

/**
 * Clip (limit) the values of a tensor. Given some interval of values, set
 * values outside of that interval to be the boundaries of the interval. All
 * values larger than the max become the max, and all values smaller than the
 * min become the min.
 *
 * TODO: consider requiring broadcasting behavior/enforcing in a test
 *
 * @param[in] tensor the tensor to clip
 * @param[in] low a tensor containing
 * @param[in] high
 * @return a tensor with all values clipped between high and low
 */
Tensor clip(const Tensor& tensor, const Tensor& low, const Tensor& high);
Tensor clip(const Tensor& tensor, const Tensor& low, const double& high);
Tensor clip(const Tensor& tensor, const double& low, const Tensor& high);
Tensor clip(const Tensor& tensor, const double& low, const double& high);

/**
 * Returns a boolean tensor which is true where the input tensor was NaN, and
 * false otherwise.
 *
 * @param[in] tensor the input tensor
 * @return a boolean array with true in positions that contained NaN in the
 * input tensor
 */
Tensor isnan(const Tensor& tensor);

/************************** Binary Operators ***************************/
#define BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, TYPE) \
  Tensor FUNC(TYPE lhs, const Tensor& rhs);         \
  Tensor FUNC(const Tensor& lhs, TYPE rhs);         \
  Tensor operator OP(TYPE lhs, const Tensor& rhs);  \
  Tensor operator OP(const Tensor& lhs, TYPE rhs);
#define BINARY_OP_LITERALS_DECL(OP, FUNC)                           \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const bool&);               \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const int&);                \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const unsigned&);           \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const char&);               \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const unsigned char&);      \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const long&);               \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const unsigned long&);      \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const long long&);          \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const unsigned long long&); \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const double&);             \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const float&);              \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const short&);              \
  BINARY_OP_LITERAL_TYPE_DECL(OP, FUNC, const unsigned short&);
#define BINARY_OP_DECL(OP, FUNC)                                    \
  Tensor FUNC(const Tensor& lhs, const Tensor& rhs);                \
  inline Tensor operator OP(const Tensor& lhs, const Tensor& rhs) { \
    return FUNC(lhs, rhs);                                          \
  }                                                                 \
  BINARY_OP_LITERALS_DECL(OP, FUNC);
// Definitions. Each declares:
// - a functional operator that takes two tensors (e.g. add)
// - a symbolic operator that takes two tensors (calls the functional one)
// - functional and symbolic operators for all lhs/rhs primitive types
BINARY_OP_DECL(+, add);
BINARY_OP_DECL(-, sub);
BINARY_OP_DECL(*, mul);
BINARY_OP_DECL(/, div);
BINARY_OP_DECL(==, eq);
BINARY_OP_DECL(!=, neq);
BINARY_OP_DECL(<, lessThan);
BINARY_OP_DECL(<=, lessThanEqual);
BINARY_OP_DECL(>, greaterThan);
BINARY_OP_DECL(>=, greaterThanEqual);
BINARY_OP_DECL(||, logicalOr);
BINARY_OP_DECL(&&, logicalAnd);
BINARY_OP_DECL(%, mod);
BINARY_OP_DECL(|, bitwiseOr);
BINARY_OP_DECL(^, bitwiseXor);
BINARY_OP_DECL(<<, lShift);
BINARY_OP_DECL(>>, rShift);

/**
 * Returns the element-wise minimum of tensor elements.
 *
 * TODO: consider requiring broadcasting behavior/enforcing in a test
 *
 * @param[in] lhs left hand side tensor for the minimum
 * @param[in] rhs right hand side tensor for the minimum
 * @return a tensor containing the minimum values in each tensor
 */
Tensor minimum(const Tensor& lhs, const Tensor& rhs);
Tensor minimum(const Tensor& lhs, const double& rhs);
Tensor minimum(const double& lhs, const Tensor& rhs);

/**
 * Returns the element-wise maximum of tensor elements.
 *
 * TODO: consider requiring broadcasting behavior/enforcing in a test
 *
 * @param[in] lhs left hand side tensor for the minimum
 * @param[in] rhs right hand side tensor for the minimum
 * @return a tensor containing the maximum values in each tensor
 */
Tensor maximum(const Tensor& lhs, const Tensor& rhs);
Tensor maximum(const Tensor& lhs, const double& rhs);
Tensor maximum(const double& lhs, const Tensor& rhs);

/**
 * Returns the element-wise exponentiation of tensors; the left hand tensor is
 * exponentiated to the power of the right hand tensor, element-wise.
 *
 * @param[in] lhs the base tensor
 * @param[in] rhs the exponent tensor
 * @return a tensor containing the exponentiated values
 */
Tensor power(const Tensor& lhs, const Tensor& rhs);

/************************** Reductions ***************************/

/**
 * Compute the minimum value along multiple axes.
 *
 * @param[in] input the input along which to operate
 * @param[in] dim the dimension along which to reduce.
 * @return a tensor containing the minimum values
 */
Tensor amin(const Tensor& input, const std::vector<int>& axes);

/**
 * Compute the maximum value along multiple axes.
 *
 * @param[in] input the input along which to operate
 * @param[in] dim the dimension along which to reduce.
 * @return a tensor containing the max
 *
 */
Tensor amax(const Tensor& input, const std::vector<int>& axes);

/**
 * Sum of array over given axes.
 *
 * @param[in] input the input along which to operate
 * @param[in] axes the dimension along which to reduce.
 * @return a tensor containing the sum across given axes
 */
Tensor sum(const Tensor& input, const std::vector<int>& axes);

/**
 * Mean of array over given axes.
 *
 * @param[in] input the input along which to operate
 * @param[in] axes the dimension along which to reduce.
 * @return a tensor containing the mean across given axes
 */
Tensor mean(const Tensor& input, const std::vector<int>& axes);

/**
 * var of array over given axes.
 *
 * @param[in] input the input along which to operate
 * @param[in] axes the dimension along which to reduce.
 * @return a tensor containing the var across given axes
 */
Tensor
var(const Tensor& input, const std::vector<int>& axes, bool bias = false);

/**
 * norm of array over all axes.
 *
 * @param[in] input the input along which to operate
 * @return a double containing the norm
 */
double norm(const Tensor& input);

} // namespace fl
