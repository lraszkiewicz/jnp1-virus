// Łukasz Raszkiewicz, Adrian Akerman

#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>
#include <queue>

class VirusNotFound : public std::exception {
	const char* what() const noexcept {
		return "VirusNotFound";
	}
};

class VirusAlreadyCreated : public std::exception {
	const char* what() const noexcept {
		return "VirusAlreadyCreated";
	}
};
class TriedToRemoveStemVirus : public std::exception {
	const char* what() const noexcept {
		return "TriedToRemoveStemVirus";
	}
};

template <class Virus>
class VirusGenealogy {

	typedef typename Virus::id_type id_type;
	typedef std::map< id_type, std::set<id_type> > dependency_map;
	typedef std::map< id_type, std::unique_ptr<Virus> > virus_ptr_map;

public:
	VirusGenealogy(const id_type & stem_id) : stem_id_(stem_id) {
		viruses_[stem_id] = std::make_unique<Virus>(stem_id);
	}

	VirusGenealogy(const VirusGenealogy &) = delete;
	VirusGenealogy & operator = (const VirusGenealogy &) = delete;

	id_type get_stem_id() const {
		return stem_id_;
	}

	std::vector<id_type> get_children(const id_type & id) const {
		return get_dependent_viruses(id, sons_);
	}

	std::vector<id_type> get_parents(const id_type & id) const {
		return get_dependent_viruses(id, parents_);
	}

	bool exists(const id_type & id) const {
		return (viruses_.find(id) != viruses_.end());
	}

	Virus & operator[] (const id_type & id) const {
		try {
			return *viruses_.at(id);
		}
		catch (const std::out_of_range & oor) {
			throw VirusNotFound();
		}
	}

	void create(const id_type & id, const id_type & parent_id) {
		if (viruses_.find(id) != viruses_.end())
			throw VirusAlreadyCreated();
		if (viruses_.find(parent_id) == viruses_.end())
			throw VirusNotFound();

		std::set<id_type> local_parents;
		std::set<id_type> local_sons = sons_[parent_id];

		std::set<id_type> & its_parents = parents_[id];
		std::set<id_type> & its_sons = sons_[parent_id];

		std::unique_ptr<Virus> virus = std::make_unique<Virus>(id);

		local_parents.insert(parent_id);
		local_sons.insert(id);

		viruses_.insert(std::make_pair(id, std::move(virus)));

		its_parents = std::move(local_parents);
		its_sons = std::move(local_sons);
	}

	void create(const id_type & id, const std::vector<id_type> & parent_ids) {
		if (viruses_.find(id) != viruses_.end())
			throw VirusAlreadyCreated();
		for (auto it = parent_ids.begin(); it != parent_ids.end(); ++it)
			if (viruses_.find(*it) == viruses_.end())
				throw VirusNotFound();

		std::set<id_type> local_parents;
		std::vector< std::set<id_type> > local_sons;

		std::set<id_type> & its_parents = parents_[id];
		std::vector< std::reference_wrapper< std::set<id_type> > > its_sons;

		size_t n = parent_ids.size();
		for (size_t i = 0; i < n; ++i) {
			local_sons.push_back(sons_[parent_ids[i]]);
			local_sons[i].insert(id);
			its_sons.push_back(sons_[parent_ids[i]]);
			local_parents.insert(parent_ids[i]);
		}

		std::unique_ptr<Virus> virus = std::make_unique<Virus>(id);

		viruses_.insert(std::make_pair(id, std::move(virus)));

		its_parents = std::move(local_parents);
		for (size_t i = 0; i < n; ++i)
			its_sons[i].get() = std::move(local_sons[i]);
	}

	void connect(const id_type & child_id, const id_type & parent_id) {
		if (viruses_.find(child_id) == viruses_.end()
				|| viruses_.find(parent_id) == viruses_.end())
			throw VirusNotFound();

		std::set<id_type> local_sons = sons_[parent_id];
		std::set<id_type> & ref_sons = sons_[parent_id];
		std::set<id_type> local_parents = parents_[child_id];
		std::set<id_type> & ref_parents = parents_[child_id];

		local_sons.insert(child_id);
		local_parents.insert(parent_id);

		ref_sons = std::move(local_sons);
		ref_parents = std::move(local_parents);
	}

	void remove(const id_type & id) {
		if (viruses_.find(id) == viruses_.end())
			throw VirusNotFound();
		if (id == stem_id_)
			throw TriedToRemoveStemVirus();

		dependency_map sons_copy = sons_;
		dependency_map parents_copy = parents_;

		std::set<id_type> viruses_to_remove;
		viruses_to_remove.insert(id);
		std::queue<id_type> virus_queue;
		std::set<id_type> queue_set;
		virus_queue.push(id);
		queue_set.insert(id);

		while (!virus_queue.empty()) {
			id_type virus_id = virus_queue.front();
			virus_queue.pop();
			queue_set.erase(virus_id);

			if (parents_copy[virus_id].empty())
				viruses_to_remove.insert(virus_id);

			if (viruses_to_remove.find(virus_id) != viruses_to_remove.end()) {
				for (id_type parent : parents_copy[virus_id])
					sons_copy[parent].erase(virus_id);

				for (id_type son : sons_copy[virus_id]) {
					if (queue_set.find(son) == queue_set.end()) {
						virus_queue.push(son);
						queue_set.insert(son);
					}
					parents_copy[son].erase(virus_id);
				}
				sons_copy.erase(virus_id);
				parents_copy.erase(virus_id);
			}
		}

		std::vector<typename virus_ptr_map::iterator> viruses_to_remove_it;
		for (id_type virus_id : viruses_to_remove)
			viruses_to_remove_it.push_back(viruses_.find(virus_id));

		for (auto it : viruses_to_remove_it)
			viruses_.erase(it);
		sons_ = std::move(sons_copy);
		parents_ = std::move(parents_copy);
	}

private:
	id_type stem_id_;
	virus_ptr_map viruses_;
	dependency_map sons_;
	dependency_map parents_;

	std::vector<id_type> get_dependent_viruses(
			const id_type & id, const dependency_map & dependency) const {
		if (viruses_.find(id) != viruses_.end()) {
			std::vector<id_type> result;

			if (dependency.find(id) != dependency.end()) {
				const std::set<id_type> & dependency_at_id = dependency.at(id);
				result.resize(dependency_at_id.size());
				std::copy(
					dependency_at_id.begin(),
					dependency_at_id.end(),
					result.begin()
				);
			}

			return result;
		}
		else
			throw VirusNotFound();
	}

};

#endif /* VIRUS_GENEALOGY */
