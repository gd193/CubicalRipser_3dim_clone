/* birthday_index.h

This file is part of CubicalRipser
Copyright 2017-2018 Takeki Sudo and Kazushi Ahara.
Modified by Shizuo Kaji

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
You should have received a copy of the GNU Lesser General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define NONE 0xffff

class Cube
{
public:
	double birth;
	unsigned index;
//	short x,y,z,m;

	Cube();
	Cube(double _b, unsigned short x, unsigned short y, unsigned short z, unsigned short m);
	Cube(const Cube&);
	void copyCube(const Cube&);
	void print();
	unsigned short x();
	unsigned short y();
	unsigned short z();
	unsigned short m();
	bool operator==(const Cube& rhs) const;
};

struct CubeComparator{
	bool operator()(const Cube& o1, const Cube& o2) const;
};
