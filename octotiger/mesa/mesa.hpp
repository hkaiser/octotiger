//  Copyright (c) 2019 Dominic C Marcello
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <string>
#include <functional>


std::function<double(double)> build_rho_of_h_from_mesa(
		const std::string& filename);
