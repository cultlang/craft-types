#include "catch2/catch.hpp"

#include "shared.h"

#include "types/utility/graph/graph.hpp"

#include <string>

using namespace graph;
using namespace Catch::Matchers;


TEST_CASE( "graph::query() basics", "[graph::GraphQuery]" )
{
    Graph< GraphCore<std::string> > g;

    test_help::fillStrGraphWithNorse(g);

    SECTION( "graph::query() returns working query object" )
    {
        auto q = query(&g);

        CHECK(q->getGraph() == &g);
        CHECK(q->getPipeline()->countPipes() == 0);

        auto r = q.run();

        CHECK(r.size() == 0);
    }

    SECTION( "->addPipe() can add GraphQueryPipeEmpty manually" )
    {
        auto q = query(&g);

        q->getPipeline()->addPipe(std::make_unique<GraphQueryPipeEmpty<decltype(g)>>());

        CHECK(q->getGraph() == &g);
        CHECK(q->getPipeline()->countPipes() == 1);

        auto r = q.run();

        CHECK(r.size() == 0);
    }

    SECTION( ".v() can add GraphQueryStepVertex (AKA `v`) with syntax (single node)" )
    {
        auto q = query(&g)
            .v(findNode(g, "thor"));

        CHECK(q->getGraph() == &g);
        CHECK(q->getPipeline()->countPipes() == 1);
        
        auto r = q.run();

        CHECK(r.size() == 1);
    }

    SECTION( ".v() can add GraphQueryStepVertex (AKA `v`) with syntax (multiple nodes)" )
    {
        auto q = query(&g)
            .v(std::vector { findNode(g, "thor"), findNode(g, "odin"), findNode(g, "jord") });

        CHECK(q->getGraph() == &g);
        CHECK(q->getPipeline()->countPipes() == 1);
        
        auto r = q.run();

        CHECK(r.size() == 3);
    }

    SECTION( ".v() can add GraphQueryStepVertex (AKA `v`) with syntax (repeat nodes)" )
    {
        auto thor = findNode(g, "thor");
        auto q = query(&g)
            .v(std::vector { thor, thor, thor, thor, thor });

        CHECK(q->getGraph() == &g);
        CHECK(q->getPipeline()->countPipes() == 1);
        
        auto r = q.run();

        CHECK(r.size() == 5);
    }

    SECTION( "after running a query, reset must be called to modify" )
    {
        auto q = query(&g);

        q->getPipeline()->addPipe(std::make_unique<GraphQueryPipeEmpty<decltype(g)>>());

        CHECK(q->getGraph() == &g);
        CHECK(q->getPipeline()->countPipes() == 1);

        auto r = q.run();

        SECTION( "not doing so causes an exception" )
        {
            REQUIRE_THROWS_AS( q->getPipeline()->addPipe(std::make_unique<GraphQueryPipeEmpty<decltype(g)>>()), graph_error );
        }

        q->reset();
        q->getPipeline()->addPipe(std::make_unique<GraphQueryPipeEmpty<decltype(g)>>());

        r = q.run();
    }
}


TEST_CASE( "graph::query() syntax traversal queries", "[graph::GraphQuery]" )
{
    Graph< GraphCore<std::string> > g;

    test_help::fillStrGraphWithNorse(g);

    SECTION( ".e() can query for specific edges." )
    {
        auto q = query(&g)
            .v(findNode(g, "thor"))
            // same as "out" (mostly)
            .e( [](auto n, auto e) { return edgeIsOutgoing<decltype(g)>(n, e) && e->data == "parents"; } );

        CHECK(q->getPipeline()->countPipes() == 2);
        
        auto r = q.run();

        CHECK(r.size() == 2); // Thor has 2 parents
    }

    SECTION( ".e() can follow edges in a specific way." )
    {
        auto q_mom = query(&g)
            .v(findNode(g, "thor"))
            // same as "out" (mostly)
            .e( [](auto n, auto e) { return edgeIsOutgoing<decltype(g)>(n, e) && e->data == "parents"; },
                // mother is in slot 1 (size guard not needed: 2 size edges are guarnteed)
                [](auto n, auto e, auto ne) { return e->nodes[1] == ne; });

        auto q_dad = query(&g)
            .v(findNode(g, "thor"))
            // same as "out" (mostly)
            .e( [](auto n, auto e) { return edgeIsOutgoing<decltype(g)>(n, e) && e->data == "parents"; },
                // father is in slot 2 (size guard requried)
                [](auto n, auto e, auto ne) { return e->nodes.size() > 2 && e->nodes[2] == ne; });

        CHECK(q_mom->getPipeline()->countPipes() == 2);
        CHECK(q_dad->getPipeline()->countPipes() == 2);
        
        auto r_mom = q_mom.run();
        auto r_dad = q_dad.run();

        REQUIRE(r_mom.size() == 1);
        REQUIRE(r_mom[0]->data == "jord"); // Thor's mom

        REQUIRE(r_dad.size() == 1);
        REQUIRE(r_dad[0]->data == "odin"); // Thor's dad
    }

    SECTION( ".in() can get nodes of incoming edges." )
    {
        auto q = query(&g)
            .v(findNode(g, "thor"))
            .in( [](auto n, auto e) { return e->data == "parents"; } );

        CHECK(q->getPipeline()->countPipes() == 2);
        
        auto r = q.run();

        CHECK(r.size() == 3); // Thor has 3 children
    }

    SECTION( ".out() can get nodes of outgoing edges." )
    {
        auto q = query(&g)
            .v(findNode(g, "thor"))
            .out( [](auto n, auto e) { return e->data == "parents"; } );

        CHECK(q->getPipeline()->countPipes() == 2);
        
        auto r = q.run();

        CHECK(r.size() == 2); // Thor has 2 parents
    }

    SECTION( ".filter() can filter nodes (1 arg, contrived)." )
    {
        auto q = query(&g)
            .v(std::vector { findNode(g, "thor"), findNode(g, "odin"), findNode(g, "odr") })
            .filter( [](auto n) { return n->data[0] != 'o'; } );

        CHECK(q->getPipeline()->countPipes() == 2);
        
        auto r = q.run();

        REQUIRE(r.size() == 1);
        CHECK(r[0]->data == "thor");
    }

    SECTION( ".filter() can filter gremlins (2 arg, contrived)." )
    {
        auto q = query(&g)
            .v(std::vector { findNode(g, "thor"), findNode(g, "odin"), findNode(g, "odr") })
            .filter( [](auto n, auto g) { return g->node->data[0] != 'o'; } );

        CHECK(q->getPipeline()->countPipes() == 2);
        
        auto r = q.run();

        REQUIRE(r.size() == 1);
        CHECK(r[0]->data == "thor");
    }

    SECTION( ".unique() ensures unique results (contrived)" )
    {
        auto thor = findNode(g, "thor");
        auto q = query(&g)
            .v(std::vector { thor, thor, thor, thor, thor })
            .unique();

        CHECK(q->getPipeline()->countPipes() == 2);

        auto r = q.run();

        REQUIRE(r.size() == 1); // Thor is unique
    }

    SECTION( ".unique() ensures unique results (large query)" )
    {
        auto q = query(&g)
            .v(findNode(g, "thor"))
            .out( [](auto n, auto e) { return e->data == "parents"; } )
            .in( [](auto n, auto e) { return e->data == "parents"; } );

        CHECK(q->getPipeline()->countPipes() == 3);

        auto r0 = q.run();

        q->reset();
        q = q.unique();

        CHECK(q->getPipeline()->countPipes() == 4);

        auto r1 = q.run();

        REQUIRE(r0.size() > r1.size());
        REQUIRE(r1.size() == 4); // Thor has 4 siblings (including himself)
    }
}

TEST_CASE( "graph::query() syntax label queries", "[graph::GraphQuery]" )
{
    Graph< GraphCore<std::string> > g;

    test_help::fillStrGraphWithNorse(g);

    SECTION( ".as() can label nodes (contrived)" )
    {
        auto q = query(&g)
            .v(findNode(g, "thor"))
            .as("me");

        CHECK(q->getPipeline()->countPipes() == 2);

        auto r = q.run();

        REQUIRE(r.size() == 1); // Thor is returned
    }

    SECTION( ".except() can filter based on labels (large query)" )
    {
        auto q = query(&g)
            .v(findNode(g, "thor"))
            .as("me")
            .out( [](auto n, auto e) { return e->data == "parents"; } )
            .in( [](auto n, auto e) { return e->data == "parents"; } )
            .unique()
            .except("me");

        CHECK(q->getPipeline()->countPipes() == 6);

        auto r = q.run();

        REQUIRE(r.size() == 3); // Thor has 3 siblings
    }

    SECTION( ".back() can allow backtracking (large query)" )
    {
        auto q = query(&g)
            .v(findNode(g, "fjorgynn"))
            .in( [](auto n, auto e) { return e->data == "parents"; } )
                .as("me")
                    .in( [](auto n, auto e) { return e->data == "parents"; } )
                    .out( [](auto n, auto e) { return e->data == "parents"; } )
                    .out( [](auto n, auto e) { return e->data == "parents"; } )
                    .filter( [](auto n) { return n->data == "bestla"; } )
            .back("me").unique()
            ;

        auto r = q.run();

        REQUIRE(r.size() == 1);
        CHECK(r[0]->data == "frigg"); // frigg is the daughter of fjorgynn who had children with one of bestla's sons
    }
}