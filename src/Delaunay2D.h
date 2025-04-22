#ifndef DELAUNAY2D_H
#define DELAUNAY2D_H

#include <vector>
#include <algorithm>
#include <unordered_set>
#include <memory>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>

class Delaunay2D {
    public:
        class DelaunayVertex {
            public:
                glm::vec3 position;

                DelaunayVertex() {}
                DelaunayVertex(const glm::vec2&pos) : position(pos.x, pos.y, 0.0f) {}
                DelaunayVertex(float x, float y) : position(x, y, 0.0f) {}

                bool operator==(const DelaunayVertex& other) const {
                    return AlmostEqual(position.x, other.position.x) &&
                            AlmostEqual(position.y, other.position.y);
                }

        };

        class DelaunayTriangle {
            public:
                DelaunayVertex a;
                DelaunayVertex b;
                DelaunayVertex c;
                bool isBad = false;

                DelaunayTriangle() : isBad(false) {}
                DelaunayTriangle(const DelaunayVertex& a, const DelaunayVertex& b, const DelaunayVertex& c)
                    : a(a), b(b), c(c), isBad(false) {}

                bool containsDelaunayVertex(const glm::vec3& v) const;
                bool circumCircleContains(const glm::vec3& v) const;
                bool operator==(const DelaunayTriangle& other) const {
                    return (a == other.a && b == other.b && c == other.c) ||
                           (a == other.b && b == other.c && c == other.a) ||
                           (a == other.c && b == other.a && c == other.b);
                }

        };

        struct DelaunayVertexHash {
            size_t operator()(const DelaunayVertex& v) const {
                return std::hash<float>()(v.position.x) ^
                       std::hash<float>()(v.position.y) ^
                       std::hash<float>()(v.position.z);
            }
        };

        class DelaunayEdge {
            public:
                DelaunayVertex u;
                DelaunayVertex v;
                bool isBad;

                DelaunayEdge() : isBad(false) {}
                DelaunayEdge(const DelaunayVertex& u, const DelaunayVertex& v)
                    : u(u), v(v), isBad(false) {}
                bool operator==(const DelaunayEdge& other) const {
                    return (u == other.u && v == other.v) || (u == other.v && v == other.u);
                }
                static bool almostEqual(const DelaunayEdge& left, const DelaunayEdge& right) {
                    return (AlmostEqual(left.u, right.u) && AlmostEqual(left.v, right.v)) ||
                           (AlmostEqual(left.u, right.v) && AlmostEqual(left.v, right.u));
                }
        };

        struct DelaunayEdgeHash {
            size_t operator()(const DelaunayEdge& edge) const {
                DelaunayVertexHash vertexHash;
                return vertexHash(edge.u) ^ vertexHash(edge.v);
            }
        };

        static bool AlmostEqual(float x, float y) {
            return std::abs(x - y) <= std::numeric_limits<float>::epsilon() * std::abs(x + y) * 2 ||
                     std::abs(x - y) < std::numeric_limits<float>::min();
        }
        static bool AlmostEqual(const DelaunayVertex& left, const DelaunayVertex& right) {
            return AlmostEqual(left.position.x, right.position.x) &&
                   AlmostEqual(left.position.y, right.position.y);
        }

        std::vector<DelaunayVertex> vertices;
        std::vector<DelaunayEdge> edges;
        std::vector<DelaunayTriangle> triangles;

        Delaunay2D() {}

        static Delaunay2D triangulate(const std::vector<DelaunayVertex>& vertices) {
            Delaunay2D delaunay;
            delaunay.vertices = vertices;
            delaunay.triangulate();
            return delaunay;
        }

    private:
        void triangulate();

};

#endif // DELAUNAY2D_H