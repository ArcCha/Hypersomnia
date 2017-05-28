#pragma once
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/memcpy_safety.h"
#include "augs/templates/dynamic_dispatch.h"
#include "augs/ensure.h"

namespace augs {
	namespace detail {
		template<size_t... _Vals>
		struct _maximum;

		template<>
		struct _maximum<>
		{	// maximum of nothing is 0
			static constexpr size_t value = 0;
		};

		template<size_t _Val>
		struct _maximum<_Val>
		{	// maximum of _Val is _Val
			static constexpr size_t value = _Val;
		};

		template<size_t _First,
			size_t _Second,
			size_t... _Rest>
			struct _maximum<_First, _Second, _Rest...>
			: _maximum<(_First < _Second ? _Second : _First), _Rest...>
		{	// find maximum value in _First, _Second, _Rest...
		};

	}

	template<class... Types>
	class trivial_variant {
		unsigned current_type = sizeof...(Types);
		char buf[detail::_maximum<0, sizeof(Types)...>::value];
		static_assert(are_types_memcpy_safe_v<Types...>, "Types must be memcpy-safe!");

	public:
		template<class T>
		static void assert_correct_type() {
			static_assert(is_one_of_v<T, Types...>, "trivial_variant does not contain the specified type!");
		}

		trivial_variant() {
			std::memset(buf, 0, sizeof(buf));
		}

		template<class T_convertible>
		trivial_variant(const T_convertible& obj) : trivial_variant() {
			set(obj);
		}

		void unset() {
			current_type = sizeof...(Types);
		}

		bool is_set() const {
			return current_type != sizeof...(Types);
		}

		template<class T_convertible>
		void set(const T_convertible& t) {
			typedef find_convertible_type_in_t<T_convertible, Types...> T;

			auto converted = T(t);
			std::memcpy(buf, &converted, sizeof(T));
			current_type = static_cast<unsigned>(index_in_v<T, Types...>);
		}

		template<class T_convertible>
		bool operator==(const T_convertible& b) const {
			typedef find_convertible_type_in_t<T_convertible, Types...> T;
			return is<T>() && get<T>() == b;
		}

		template<class T>
		bool is() const {
			assert_correct_type<T>();
			return index_in_v<T, Types...> == static_cast<std::size_t>(current_type);
		}

		template <std::size_t I>
		auto& get() {
			return get<nth_type_in_t<I, Types...>>();
		}

		template <std::size_t I>
		const auto& get() const {
			return get<nth_type_in_t<I, Types...>>();
		}

		template <class T>
		T& get() {
			assert_correct_type<T>();
			ensure(is<T>());
			return *reinterpret_cast<T*>(buf);
		}

		template <class T>
		const T& get() const {
			assert_correct_type<T>();
			ensure(is<T>());
			return *reinterpret_cast<const T*>(buf);
		}

		template<class T>
		T* find() {
			assert_correct_type<T>();
			return is<T>() ? reinterpret_cast<T*>(buf) : nullptr;
		}

		template<class T>
		const T* find() const {
			assert_correct_type<T>();
			return is<T>() ? reinterpret_cast<T*>(buf) : nullptr;
		}

		template<class L>
		decltype(auto) call(L&& generic_call) {
			return dynamic_dispatch(*this, current_type, std::forward<L>(generic_call));
		}

		template<class L>
		decltype(auto) call(L&& generic_call) const {
			return dynamic_dispatch(*this, current_type, std::forward<L>(generic_call));
		}

		unsigned get_current_type_index() const {
			return current_type;
		}

		bool operator==(const trivial_variant& b) const {
			return 
				current_type == b.current_type 
				&& call(
					[&](const auto& resolved_a) {
						return resolved_a == b.get<std::decay_t<decltype(resolved_a)>>();
					}
				)
			;
		}
	};
}

namespace std {
	template<std::size_t I, class... Types>
	decltype(auto) get(augs::trivial_variant<Types...>& t) {
		return t.get<I>();
	}

	template<std::size_t I, class... Types>
	decltype(auto) get(const augs::trivial_variant<Types...>& t) {
		return t.get<I>();
	}

	template<class T, class... Types>
	decltype(auto) get(augs::trivial_variant<Types...>& t) {
		return t.get<T>();
	}

	template<class T, class... Types>
	decltype(auto) get(const augs::trivial_variant<Types...>& t) {
		return t.get<T>();
	}

	template <class... Types>
	struct hash<augs::trivial_variant<Types...>> {
		std::size_t operator()(const augs::trivial_variant<Types...>& k) const {
			return k.call([&k](const auto& resolved) {
				return augs::simple_two_hash(k.get_current_type_index(), resolved);
			});
		}
	};
}