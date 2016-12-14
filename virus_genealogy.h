#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

class VirusNotFound : public std::exception {};
class VirusAlreadyCreated : public std::exception {};
class TriedToRemoveStemVirus : public std::exception {};

template <class Virus>
class VirusGenealogy {

	typedef typename Virus::id_type id_type;
	typedef std::map< id_type, std::set<id_type> > dependency_map;

public:
	VirusGenealogy(const id_type & stem_id) : stem_id(stem_id) {
		viruses[stem_id] = std::make_unique<Virus>(stem_id);
	}

	id_type get_stem_id() const {
		return stem_id;
	}

	std::vector<id_type> get_children(const id_type & id) const {
		return get_dependent_viruses(id, sons);
	}

	std::vector<id_type> get_parents(const id_type & id) const {
		return get_dependent_viruses(id, parents);
	}

	bool exists(const id_type & id) const {
		return (viruses.find(id) != viruses.end());
	}

	Virus & operator[] (const id_type & id) const {
		try {
			return *viruses.at(id);
		}
		catch (const std::out_of_range & oor) {
			throw VirusNotFound();
		}
	}

	void create(const id_type & id, const id_type & parent_id) {
		if (viruses.find(id) != viruses.end())
			throw VirusAlreadyCreated();
		if (viruses.find(parent_id) == viruses.end())
			throw VirusNotFound();
			
		std::set<id_type> empty_set;
		
		std::set<id_type> local_parents;
		std::set<id_type> local_sons = sons[parent_id];
	
		std::set<id_type> & its_sons = sons[parent_id];
		
		std::unique_ptr<Virus> virus = std::make_unique<Virus>(id);
		
		local_parents.insert(parent_id);
		local_sons.insert(id);
		
		if (parents.find(id) != parents.end())
			parents.insert(std::make_pair(id, empty_set));
		
		std::set<id_type> & its_parents = parents[id];
		
		viruses.insert(std::make_pair(id, std::move(virus)));
		
		its_parents = std::move(local_parents);
		its_sons = std::move(local_sons);
	}

	void create(const id_type & id, const std::vector<id_type> & parent_ids) {
		if (viruses.find(id) != viruses.end())
			throw VirusAlreadyCreated();
		for (auto it = parent_ids.begin(); it != parent_ids.end(); ++it)
			if (viruses.find(*it) == viruses.end())
				throw VirusNotFound();
		// TODO
	}

	void connect(const id_type & child_id, const id_type & parent_id) {
		if (viruses.find(child_id) == viruses.end()
				|| viruses.find(parent_id) == viruses.end())
			throw VirusNotFound();
		// TODO
	}

	void remove(const id_type & id) {
		if (viruses.find(id) == viruses.end())
			throw VirusNotFound();
		if (id == stem_id)
			throw TriedToRemoveStemVirus();
		// TODO
	}

private:
	id_type stem_id;
	std::map<id_type, std::unique_ptr<Virus>> viruses;
	dependency_map sons;
	dependency_map parents;

	std::vector<id_type> get_dependent_viruses(
			const id_type & id, const dependency_map & dependency) const {
		if (viruses.find(id) != viruses.end()) {
			std::vector<id_type> result;

			if (dependency.find(id) != dependency.end()) {
				const std::set<id_type> & dependency_at_id = dependency.at(id);
				result.resize(dependency_at_id.size());
				std::copy(dependency_at_id.begin(),
						  dependency_at_id.end(),
					      result.begin());
			}

			return result;
		}
		else
			throw VirusNotFound();
	}

};

#endif /* VIRUS_GENEALOGY */
