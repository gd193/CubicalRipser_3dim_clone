/*
This file is part of CubicalRipser
Copyright 2017-2018 Takeki Sudo and Kazushi Ahara.
Modified by Shizuo Kaji

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
You should have received a copy of the GNU Lesser General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

using namespace std;

#include "cube.h"
#include "dense_cubical_grids.h"
#include "coboundary_enumerator.h"
#include "write_pairs.h"
#include "joint_pairs.h"
#include "compute_pairs.h"
#include "config.h"
#include "npy.hpp"


void print_usage_and_exit(int exit_code) {
	 cerr << "Usage: "
	      << "cubicalripser "
	      << "[options] [input_filename]" << endl
	      << endl
	      << "Options:" << endl
	      << endl
	      << "  --help           print this screen" << endl
	      << "  --threshold <t>  compute cubical complexes up to birth time <t>" << endl
		  << "  --maxdim <t>     compute persistent homology up to dimension <t>" << endl
	      << "  --method         method to compute the persistent homology of the cubical complexes. Options are" << endl
	      << "                     link_find      (calculating the 0-dim PH by the 'link_find' algorithm; default)" << endl
	      << "                     compute_pairs  (calculating the 0-dim PH by the 'compute_pairs' algorithm)" << endl
	      << "  --min_cache_size  minimum number of non-zero entries of a reduced column to be cached (the higher the slower but less memory)" << endl
	      << "  --output         name of file that will contain the persistence diagram " << endl
	      << "  --print          print persistence pairs on your console" << endl
	      << "  --top_dim        compute only for top dimension using Alexander duality" << endl
	      << "  --location       output type of location" << endl
	      << "                     birth      (localtion of birth cell; default)" << endl
	      << "                     death      (localtion of death cell)" << endl
	      << "                     none      (output nothing)" << endl
	      << endl;

	exit(exit_code);
}

/////////////////////////////////////////////
int main(int argc, char** argv){

	Config config;
	// command-line argument parsing
	for (int i = 1; i < argc; ++i) {
		const string arg(argv[i]);
		if (arg == "--help") {
			print_usage_and_exit(0);
		} else if (arg == "--threshold") {
			string parameter = string(argv[++i]);
			size_t next_pos;
			config.threshold = stod(parameter, &next_pos);
			if (next_pos != parameter.size()) print_usage_and_exit(-1);
		} else if (arg == "--maxdim") {
			config.maxdim = stoi(argv[++i]);
		} else if(arg == "--method") {
			string parameter = string(argv[++i]);
			if (parameter == "link_find") {
				config.method = LINKFIND;
			} else if (parameter == "compute_pairs") {
				config.method = COMPUTEPAIRS;
			} else {
				print_usage_and_exit(-1);
			}
		} else if (arg == "--output") {
			config.output_filename = string(argv[++i]);
		} else if (arg == "--min_cache_size"){
            config.min_cache_size = stoi(argv[++i]);
		} else if (arg == "--print"){
			config.print = true;
		} else if (arg == "--top_dim") {
			config.method = ALEXANDER;
		} else if (arg == "--location"){
			string parameter = string(argv[++i]);
			if (parameter == "birth") {
				config.location = LOC_BIRTH;
			} else if (parameter == "death") {
				config.location = LOC_DEATH;
			} else if (parameter == "none") {
				config.location = LOC_NONE;
			} else {
				print_usage_and_exit(-1);
			}
		} else {
			if (!config.filename.empty()) { print_usage_and_exit(-1); }
			config.filename = argv[i];
		}
	}

	if (config.filename.empty()) { print_usage_and_exit(-1); }
    ifstream file_stream(config.filename);
	if (!config.filename.empty() && file_stream.fail()) {
		cerr << "couldn't open file " << config.filename << endl;
		exit(-1);
	}
	// infer input file type from its extention
	if(config.filename.find(".txt")!= std::string::npos){
		config.format = PERSEUS;
	}else if(config.filename.find(".npy")!= std::string::npos){
		config.format = NUMPY;
	}else if(config.filename.find(".complex")!= std::string::npos){
		config.format = DIPHA;
	}else{
		cerr << "unknown input file format! (the filename extension should be one of npy, txt, complex): " << config.filename << endl;
		exit(-1);
	}

	vector<WritePairs> writepairs; // record (dim birth death x y z)
	writepairs.clear();
	
	DenseCubicalGrids* dcg = new DenseCubicalGrids(config);
	vector<Cube> ctr;

	// compute PH
    vector<uint32_t> betti(0);
	switch(config.method){
		case LINKFIND:
		{
			dcg->loadImage(false);
			config.maxdim = std::min<uint8_t>(config.maxdim, dcg->dim - 1);
			JointPairs* jp = new JointPairs(dcg, writepairs, config);
			if(dcg->dim==1){
				jp -> enum_edges({0},ctr);
			}else if(dcg->dim==2){
				jp -> enum_edges({0,1},ctr);
			}else{
				jp -> enum_edges({0,1,2},ctr);
			}
			jp -> joint_pairs_main(ctr,0); // dim0
            betti.push_back(writepairs.size());
            cout << "the number of pairs in dim 0: " << betti[0] << endl;
			if(config.maxdim>0){
				ComputePairs* cp = new ComputePairs(dcg, writepairs, config);
				cp -> compute_pairs_main(ctr); // dim1
                betti.push_back(writepairs.size() - betti[0]);
                cout << "the number of pairs in dim 1: " << betti[1] << endl;
				if(config.maxdim>1){
					cp -> assemble_columns_to_reduce(ctr,2);
					cp -> compute_pairs_main(ctr); // dim2
                    betti.push_back(writepairs.size() - betti[0] - betti[1]);
                    cout << "the number of pairs in dim 2: " << betti[2] << endl;
				}
			}
		break;
		}
		
		case COMPUTEPAIRS:
		{
			dcg->loadImage(false);
			config.maxdim = std::min<uint8_t>(config.maxdim, dcg->dim - 1);
			ComputePairs* cp = new ComputePairs(dcg, writepairs, config);
			cp -> assemble_columns_to_reduce(ctr,0);
			cp -> compute_pairs_main(ctr); // dim0
            betti.push_back(writepairs.size());
            cout << "the number of pairs in dim 0: " << betti[0] << endl;
			if(config.maxdim>0){
				cp -> assemble_columns_to_reduce(ctr,1);
				cp -> compute_pairs_main(ctr); // dim1
				betti.push_back(writepairs.size() - betti[0]);
				cout << "the number of pairs in dim 1: " << betti[1] << endl;
				if(config.maxdim>1){
					cp -> assemble_columns_to_reduce(ctr,2);
					cp -> compute_pairs_main(ctr); // dim2
					betti.push_back(writepairs.size() - betti[0] - betti[1]);
					cout << "the number of pairs in dim 2: " << betti[2] << endl;
				}
			}
		break;
		}

		case ALEXANDER: // only for top dim
		{
			dcg->loadImage(true);
			JointPairs* jp = new JointPairs(dcg, writepairs, config);
			if(dcg->dim==1){
				jp -> enum_edges({0},ctr);
				jp -> joint_pairs_main(ctr,0); // dim0
				cout << "the number of pairs in dim 0: " << writepairs.size() << endl;
			}else if(dcg->dim==2){
				jp -> enum_edges({0,1,3,4},ctr);
				jp -> joint_pairs_main(ctr,1); // dim1
				cout << "the number of pairs in dim 1: " << writepairs.size() << endl;
			}else if(dcg->dim==3){
				jp -> enum_edges({0,1,2,3,4,5,6,7,8,9,10,11,12},ctr);
				jp -> joint_pairs_main(ctr,2); // dim2
				cout << "the number of pairs in dim 2: " << writepairs.size() << endl;
			}
		break;
		}		
	}

	// write to file
	ofstream writing_file;
	int64_t p = writepairs.size();
	cout << "the number of total pairs : " << p << endl;
	if(config.output_filename.find(".csv")!= std::string::npos){
		writing_file.open(config.output_filename, ios::out);
		if(!writing_file.is_open()){
			cerr << " error: open file for output failed! " << endl;
		}
		for(int64_t i = 0; i < p; ++i){
			int64_t d = writepairs[i].dim;
			writing_file << d << ",";
			writing_file << writepairs[i].birth << ",";
			writing_file << writepairs[i].death;
			if(config.location != LOC_NONE){
				writing_file << "," << writepairs[i].birth_x << "," << writepairs[i].birth_y<< "," << writepairs[i].birth_z;
			}
			writing_file << endl;
		}
		writing_file.close();
	}else if(config.output_filename.find(".npy")!= std::string::npos){
		long unsigned leshape[] = {0,6};
		leshape[0] = p;
		vector<double> data(6*p);
		for(int64_t i = 0; i < p; ++i){
			data[6*i] = writepairs[i].dim;
			data[6*i+1] = writepairs[i].birth;
			data[6*i+2] = writepairs[i].death;
			data[6*i+3] = writepairs[i].birth_x;
			data[6*i+4] = writepairs[i].birth_y;
			data[6*i+5] = writepairs[i].birth_z;
		}
		npy::SaveArrayAsNumpy(config.output_filename, false, 2, leshape, data);
	} else { // DIPHA format
		writing_file.open(config.output_filename, ios::out | ios::binary);
		if(!writing_file.is_open()){
			cerr << " error: open file for output failed! " << endl;
		}
		int64_t mn = 8067171840;
		writing_file.write((char *) &mn, sizeof( int64_t )); // magic number
		int64_t type = 2;
		writing_file.write((char *) &type, sizeof( int64_t )); // type number of PERSISTENCE_DIAGRAM
		writing_file.write((char *) &p, sizeof( int64_t )); // number of points in the diagram p
		for(int64_t i = 0; i < p; ++i){
			int64_t writedim = writepairs[i].dim;
			writing_file.write((char *) &writedim, sizeof( int64_t ));
			writing_file.write((char *) &writepairs[i].birth, sizeof( double ));
			writing_file.write((char *) &writepairs[i].death, sizeof( double ));
		}
		writing_file.close();
	}

	return 0;
}