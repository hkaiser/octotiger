/*
 * line_of_centers.cpp
 *
 *  Created on: Apr 20, 2016
 *      Author: dmarce1
 */

#include "../node_server.hpp"
#include "../node_client.hpp"
#include "util.hpp"

typedef node_server::line_of_centers_action line_of_centers_action_type;
HPX_REGISTER_ACTION (line_of_centers_action_type);

hpx::future<line_of_centers_t> node_client::line_of_centers(const std::pair<space_vector, space_vector>& line) const {
	return hpx::async<typename node_server::line_of_centers_action>(get_gid(), line);
}


void output_line_of_centers(FILE* fp, const line_of_centers_t& loc) {
	for (integer i = 0; i != loc.size(); ++i) {
		fprintf(fp, "%e ", loc[i].first);
		for (integer j = 0; j != NF+NGF; ++j) {
			fprintf(fp, "%e ", loc[i].second[j]);
		}
		fprintf(fp, "\n");
	}
}

line_of_centers_t node_server::line_of_centers(const std::pair<space_vector, space_vector>& line) const {
	std::list<hpx::future<line_of_centers_t>> futs;
	line_of_centers_t return_line;
	if (is_refined) {
		for (integer ci = 0; ci != NCHILD; ++ci) {
			futs.push_back(children[ci].line_of_centers(line));
		}
		std::map<real, std::vector<real>> map;
		for (auto&& fut : futs) {
			auto tmp = fut.get();
			for (integer ii = 0; ii != tmp.size(); ++ii) {
				if( map.find(tmp[ii].first) == map.end()) {
					map.emplace(std::move(tmp[ii]));
				}
			}
		}
		return_line.resize(map.size());
		std::move(map.begin(), map.end(), return_line.begin());
	} else {
		return_line = grid_ptr->line_of_centers(line);
	}

	return return_line;
}

void line_of_centers_analyze(const line_of_centers_t& loc, real omega, std::pair<real, real>& rho1_max,
		std::pair<real, real>& rho2_max, std::pair<real, real>& l1_phi,std::pair<real, real>& l2_phi,std::pair<real, real>& l3_phi, real& rho1_phi, real& rho2_phi) {

	for( auto& l : loc) {
		ASSERT_NONAN(l.first);
		for( integer f = 0; f != NF+NGF; ++f) {
			ASSERT_NONAN(l.second[f]);
		}
	}


	rho1_max.second = rho2_max.second = 0.0;
	integer rho1_maxi, rho2_maxi;
	///	printf( "LOCSIZE %i\n", loc.size());
	for (integer i = 0; i != loc.size(); ++i) {
		const real x = loc[i].first;
		const real rho = loc[i].second[rho_i];
		const real pot = loc[i].second[pot_i];
	//	printf( "%e %e\n", x, rho);
		if (rho1_max.second < rho) {
		//	printf( "!\n");
			rho1_max.second = rho;
			rho1_max.first = x;
			rho1_maxi = i;
			real phi_eff = pot / ASSERT_POSITIVE(rho) - 0.5 * x * x * omega * omega;
			rho1_phi = phi_eff;
		}
	}
	for (integer i = 0; i != loc.size(); ++i) {
		const real x = loc[i].first;
		if (x * rho1_max.first < 0.0) {
			const real rho = loc[i].second[rho_i];
			const real pot = loc[i].second[pot_i];
			if (rho2_max.second < rho) {
				rho2_max.second = rho;
				rho2_max.first = x;
				rho2_maxi = i;
				real phi_eff = pot / ASSERT_POSITIVE(rho) - 0.5 * x * x * omega * omega;
				rho2_phi = phi_eff;
			}
		}
	}
	l1_phi.second = -std::numeric_limits < real > ::max();
	l2_phi.second = -std::numeric_limits < real > ::max();
	l3_phi.second = -std::numeric_limits < real > ::max();
	for (integer i = 0; i != loc.size(); ++i) {
		const real x = loc[i].first;
		const real rho = loc[i].second[rho_i];
		const real pot = loc[i].second[pot_i];
		real phi_eff = pot / ASSERT_POSITIVE(rho) - 0.5 * x * x * omega * omega;
		if (x > std::min(rho1_max.first, rho2_max.first) && x < std::max(rho1_max.first, rho2_max.first)) {
			if (phi_eff > l1_phi.second) {
				l1_phi.second = phi_eff;
				l1_phi.first = x;
			}
		}
		else if (std::abs(x) > std::abs(rho2_max.first) && x*rho2_max.first > 0.0 ) {
			if (phi_eff > l2_phi.second) {
				l2_phi.second = phi_eff;
				l2_phi.first = x;
			}
		}
		else if (std::abs(x) > std::abs(rho1_max.first)) {
			if (phi_eff > l3_phi.second) {
				l3_phi.second = phi_eff;
				l3_phi.first = x;
			}
		}
	}
}