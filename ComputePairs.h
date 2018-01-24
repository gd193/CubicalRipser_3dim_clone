#include <vector>
#include <unordered_map>

using namespace std;

template <class Key, class T> class hash_map : public std::unordered_map<Key, T> {};

class ComputePairs
{
public:
	DenseCubicalGrids* dcg;
	ColumnsToReduce* ctr;
	hash_map<int, int> pivot_column_index;
	int ax, ay, az;
	int dim;
	int mode = 0;
	vector<WritePairs> *wp;

	ComputePairs(DenseCubicalGrids* _dcg, ColumnsToReduce* _ctr, vector<WritePairs> &_wp, int _mode);

	void compute_pairs_main();

	BirthdayIndex pop_pivot(priority_queue<BirthdayIndex, vector<BirthdayIndex>, BirthdayIndexComparator>&
		column);

	BirthdayIndex get_pivot(priority_queue<BirthdayIndex, vector<BirthdayIndex>, BirthdayIndexComparator>&
		column);

	void assemble_columns_to_reduce();
};