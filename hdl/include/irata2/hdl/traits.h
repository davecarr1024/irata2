#ifndef IRATA2_HDL_TRAITS_H
#define IRATA2_HDL_TRAITS_H

#include <type_traits>

namespace irata2::hdl {

struct BusTag {};
struct ControlTag {};
struct RegisterTag {};
struct StatusTag {};

template <typename T>
inline constexpr bool is_bus_v = std::is_base_of_v<BusTag, T>;

template <typename T>
inline constexpr bool is_control_v = std::is_base_of_v<ControlTag, T>;

template <typename T>
inline constexpr bool is_register_v = std::is_base_of_v<RegisterTag, T>;

template <typename T>
inline constexpr bool is_status_v = std::is_base_of_v<StatusTag, T>;

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_TRAITS_H
