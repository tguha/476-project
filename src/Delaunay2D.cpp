#include "Delaunay2D.h"

bool Delaunay2D::DelaunayTriangle::containsDelaunayVertex(const glm::vec3& v) const {
    return glm::distance(v, a.position) < 0.01f ||
           glm::distance(v, b.position) < 0.01f ||
           glm::distance(v, c.position) < 0.01f;
}

bool Delaunay2D::DelaunayTriangle::circumCircleContains(const glm::vec3& v) const {
    // Check if the point is inside the circumcircle of the triangle
    const glm::vec3& av = a.position;
    const glm::vec3& bv = b.position;
    const glm::vec3& cv = c.position;

    float ab = glm::dot(av, av);
    float cd = glm::dot(bv, bv);
    float ef = glm::dot(cv, cv);

    float circumX = (ab * (cv.y - bv.y) + cd * (av.y - cv.y) + ef * (bv.y - av.y)) /
                    (av.x * (cv.y - bv.y) + bv.x * (av.y - cv.y) + cv.x * (bv.y - av.y));
    float circumY = (ab * (bv.x - cv.x) + cd * (cv.x - av.x) + ef * (av.x - bv.x)) /
                    (av.y * (bv.x - cv.x) + bv.y * (cv.x - av.x) + cv.y * (av.x - bv.x));

    glm::vec3 circum(circumX / 2, circumY / 2, 0);
    float circumRadius = glm::length(av - circum);
    float dist = glm::length(v - circum);
    return dist <= circumRadius;
}

void Delaunay2D::triangulate() {
    if (vertices.empty()) return;

    float minX = vertices[0].position.x;
    float minY = vertices[0].position.y;
    float maxX = minX;
    float maxY = minY;

    // Find the bounds of the point set
    for (const auto& vertex : vertices) {
        if (vertex.position.x < minX) minX = vertex.position.x;
        if (vertex.position.x > maxX) maxX = vertex.position.x;
        if (vertex.position.y < minY) minY = vertex.position.y;
        if (vertex.position.y > maxY) maxY = vertex.position.y;
    }

    float dx = maxX - minX;
    float dy = maxY - minY;
    float delataMax = std::max(dx, dy) * 2;

    DelaunayVertex superP1(minX - 1, minY - 1);
    DelaunayVertex superP2(minX - 1, maxY + delataMax);
    DelaunayVertex superP3(maxX + delataMax, minY - 1);

    triangles.push_back(DelaunayTriangle(superP1, superP2, superP3));

    // Add points one by one
    for (const auto& vertex : vertices) {
        std::vector<DelaunayEdge> polygonEdges;

        for (auto& triangle : triangles) {
            if (triangle.circumCircleContains(vertex.position)) {
                triangle.isBad = true;
                polygonEdges.push_back(DelaunayEdge(triangle.a, triangle.b));
                polygonEdges.push_back(DelaunayEdge(triangle.b, triangle.c));
                polygonEdges.push_back(DelaunayEdge(triangle.c, triangle.a));
            }
        }

        // Remove bad triangles
        triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
            [](const DelaunayTriangle& t) { return t.isBad; }), triangles.end());

        // Find edges that are shared by two triangles
        for (size_t i = 0; i < polygonEdges.size(); ++i) {
            for (size_t j = i + 1; j < polygonEdges.size(); ++j) {
                if (polygonEdges[i] == polygonEdges[j]) {
                    polygonEdges[i].isBad = true;
                    polygonEdges[j].isBad = true;
                }
            }
        }

        // Remove bad edges
        polygonEdges.erase(std::remove_if(polygonEdges.begin(), polygonEdges.end(),
            [](const DelaunayEdge& e) { return e.isBad; }), polygonEdges.end());

        // Create new triangles form the remaining edges and the new point
        for (const auto& edge : polygonEdges) {
            triangles.push_back(DelaunayTriangle(edge.u, edge.v, vertex));
        }
    }

    // Remove any triangles with vertices from the super triangle
    triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
        [&superP1, &superP2, &superP3](const DelaunayTriangle& t) {
            return t.containsDelaunayVertex(superP1.position) ||
                   t.containsDelaunayVertex(superP2.position) ||
                   t.containsDelaunayVertex(superP3.position);
        }), triangles.end());

    // Extract unique edges from triangles
    std::unordered_set<DelaunayEdge, DelaunayEdgeHash> uniqueEdges;
    for (const auto& triangle : triangles) {
        DelaunayEdge ab(triangle.a, triangle.b);
        DelaunayEdge bc(triangle.b, triangle.c);
        DelaunayEdge ca(triangle.c, triangle.a);

        if (uniqueEdges.insert(ab).second) {
            edges.push_back(ab);
        }

        if (uniqueEdges.insert(bc).second) {
            edges.push_back(bc);
        }

        if (uniqueEdges.insert(ca).second) {
            edges.push_back(ca);
        }
    }

    std::cout << "Triangulation complete. Found " << triangles.size() << " triangles." << std::endl;
}

