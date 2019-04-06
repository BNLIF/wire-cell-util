/*
 * A graph templated on vertex type which is indexed by instances of
 * that type.  That is, unlike the underlying boost graph that it uses
 * you can treat a vertex as if it was an instance instead of going
 * through boost vertex descriptors (numbers) and vertex properties.
 * 
 * To do this, a mapping is used so it will slow down operation.  Any
 * graph operations can still make use of boost implementations.
 * 
 * See test_indexedgraph.cxx
 */

#ifndef WIRECELL_INDEXEDGRAPH
#define WIRECELL_INDEXEDGRAPH


#include <boost/graph/connected_components.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>

#include <unordered_set>
#include <set>
#include <variant>              // C++17
#include <iostream>             // debug


namespace WireCell {

    // VertexType must be hashable.
    template <typename VertexType>
    class IndexedGraph {
    public:
        typedef VertexType vertex_t;
        // the underlying graph does not allow parallel edges (setS)
        // and vertices are kept unique via the index so use vecS for
        // speed.
        typedef boost::adjacency_list<boost::setS, boost::vecS,
                                      boost::undirectedS,
                                      vertex_t> graph_t;
        typedef typename boost::graph_traits<graph_t>::vertex_descriptor vdesc_t;
        typedef typename boost::graph_traits<graph_t>::edge_descriptor edesc_t;

        IndexedGraph() {}

        IndexedGraph(const graph_t& g) {

            // can't do straight copy() with setS unless give vertex index map.
            typedef std::unordered_map<vdesc_t, size_t> vertex_map_t;
            vertex_map_t vmap;
            boost::associative_property_map<vertex_map_t> pmapindx(vmap);
            int count = 0;
            for (auto v : boost::make_iterator_range(boost::vertices(g))) {
                boost::put(pmapindx, v, count++);
            }
            boost::copy_graph(g, m_graph, boost::vertex_index_map( pmapindx ) );

            // make index
            for (auto v : boost::make_iterator_range(boost::vertices(m_graph))) {
                m_index[m_graph[v]] = v;
            }
        }

        // return vertex objects connected to the vertex of given object.
        std::vector<vertex_t> neighbors(vertex_t obj) const {
            std::vector<vertex_t> ret;
            auto it = m_index.find(obj);
            if (it == m_index.end()) {
                return ret;
            }
            vdesc_t vd = it->second;
            for (auto edge : boost::make_iterator_range(boost::out_edges(vd, m_graph))) {
                vdesc_t neigh = boost::target(edge, m_graph);
                vertex_t tmp = m_graph[neigh];
                if (!has(tmp)) {
                    std::cerr << "WTF, I don't have my own neighbors: "
                              << boost::source(edge, m_graph) << " -> " << neigh 
                              << " " << boost::num_vertices(m_graph) << " " << m_index.size()
                              << std::endl;
                }
                ret.push_back(tmp);
            }
            return ret;
        }

        // return true if graph has vertex object
        bool has(vertex_t vobj) const {
            auto it = m_index.find(vobj);
            if (it == m_index.end()) {
                return false;
            }
            return true;
        }
            
        // Add edge between two objects, return underlying edge descriptor.
        edesc_t edge(vertex_t vobj1, vertex_t vobj2) {
            vdesc_t v1 = vertex(vobj1);
            vdesc_t v2 = vertex(vobj2);
            auto ret = boost::add_edge(v1, v2, m_graph);
            return ret.first;
        }

        // Add vertex object, return underlying vertex descriptor.
        vdesc_t vertex(vertex_t vobj) {
            auto it = m_index.find(vobj);
            if (it != m_index.end()) {
                return it->second;
            }
            vdesc_t vd = boost::add_vertex(vobj, m_graph);
            m_index[vobj] = vd;
            return vd;
        }


        vdesc_t replace(vertex_t vold, vertex_t vnew) {
            auto vd = vertex(vold);
            m_graph[vd] = vnew;
            m_index[vnew] = vd;
            return vd;
        }


        // clear index and graph.
        void clear() {
            m_index.clear();
            m_graph.clear();
        }

        void dump() {
            auto be = boost::vertices(m_graph);
            std::cerr << "IndexedGraph with " << m_index.size() << " indexed and "
                      <<  (be.second-be.first) << " nodes\n";
        }

        /// Return connected component subgraphs.
        typedef std::unordered_map<int, std::vector<vertex_t> > vertex_grouping_t;
        vertex_grouping_t groups() {
            std::unordered_map<vdesc_t, int> stripes;
            boost::connected_components(m_graph,
                                        boost::make_assoc_property_map(stripes));
            vertex_grouping_t ret;
            for (auto& vg : stripes) { // invert
                ret[vg.second].push_back(m_graph[vg.first]);
            }
            return ret;
        }

        // Access underlying Boost graph, read-only.
        const graph_t& graph() const { return m_graph; }

        // Mutable access.  Any changes made will NOT be reflected in
        // the index.
        graph_t& graph() { return m_graph; }

        typedef std::unordered_map<vertex_t, vdesc_t> index_t;
        index_t& index() { return m_index; }
        const index_t& index() const { return m_index; }

    private:
        graph_t m_graph;
        index_t m_index;
    };
    
}

#endif
