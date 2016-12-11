#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include<map>
#include<set>
#include<vector>
#include<algorithm>

class VirusNotFound : public std::exception{
};

template <class Virus>
class VirusGenealogy{
	typedef typename Virus::id_type id_type;
	typedef std::map<id_type, std::set<id_type>> dependency_map;
	
	public:
		VirusGenealogy(const id_type & stem_id):
			stem_id(stem_id)
		{
			viruses.insert(Virus(stem_id));
		}
		
		id_type get_stem_id() const{
			return stem_id;
		}
		
		std::vector<id_type> get_children(const id_type & id) const
		{
			return get_dependent_viruses(id, sons);
		}
		
		std::vector<id_type> get_parents(const id_type & id) const{
			return get_dependent_viruses(id, parents);
		}
		
		bool exists(const id_type & id) const{
			return (viruses.find(id) != viruses.end());
		}
		
		Virus & operator[] (const id_type & id) const{
			if (viruses.find(id) != viruses.end())
				return viruses[id];
			else
				throw VirusNotFound();
		}
		
	private:
		id_type stem_id;
		std::map<id_type, Virus> viruses;
		dependency_map sons;
		dependency_map parents;
			
		std::vector<id_type> get_dependent_viruses(
			const id_type & id, const dependency_map & dependency) const
		{
			if (dependency.find(id) != dependency.end()){
				std::vector<id_type> result(dependency[id].size());
				std::copy(dependency[id].begin(),
					dependency[id].end(),
					result.begin());
				
				return result;
			}
			else
				throw VirusNotFound();
		}
};

#endif /* VIRUS_GENEALOGY */
