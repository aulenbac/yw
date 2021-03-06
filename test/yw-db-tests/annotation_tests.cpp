#include "yw_db_tests.h"

using namespace yw;
using namespace yw::db;
using namespace yw::test;

using std::make_unique;
using Tag = yw::db::Annotation::Tag;

YW_TEST_FIXTURE(Annotation)

    YesWorkflowDB ywdb { false };
    row_id user13;
    row_id extraction17;
    row_id file5;
    row_id source8;
    row_id line19, line25, line77, line119, line125, line177, line199;

    YW_TEST_SETUP(Annotation)
    {
        ywdb.createUserTable();
        Expect::AreEqual(13, ywdb.insert(User{ (user13 = 13), "user1" }));

        ywdb.createExtractionTable();
        Expect::AreEqual(17, ywdb.insert(Extraction{ (extraction17 = 17), user13, "2017-06-22 10:52:00.000" }));

        ywdb.createFileTable();
        Expect::AreEqual(5, ywdb.insert(File{ (file5 = 5), "main.c" }));

        ywdb.createSourceTable();
        Expect::AreEqual(8, ywdb.insert(Source{ (source8 = 8), file5, "C" }));

        ywdb.createLineTable();
        Expect::AreEqual(19, ywdb.insert(Line{ (line19 = 19), source8, 1, "@begin block" }));
        Expect::AreEqual(25, ywdb.insert(Line{ (line25 = 25), source8, 4, "@in port" }));
        Assert::AreEqual(77, ywdb.insert(Line{ (line77 = 77), source8, 5, "@end block" }));
        Expect::AreEqual(119, ywdb.insert(Line{ (line119 = 119), source8, 11, "@begin second block" }));
        Expect::AreEqual(125, ywdb.insert(Line{ (line125 = 125), source8, 14, "@in second port" }));
        Assert::AreEqual(177, ywdb.insert(Line{ (line177 = 177), source8, 15, "@end second block" }));

        ywdb.createAnnotationTable();
    }

YW_TEST_SET

    YW_TEST(Annotation, FieldValuesMatchAssignedValuesWithNulls) {
        Assert::AreEqual(
            "NULL|17|END|NULL|19|1|0|11|@end|NULL", 
            Annotation{ auto_id, 17, Tag::END, null_id, 19, 1, 0, 11, "@end", null_string }.fieldValues()
        );
    }

    YW_TEST(Annotation, FieldValuesMatchAssignedValuesWithoutNulls) {
        Assert::AreEqual(
            "2|17|BEGIN|1|19|1|0|11|@begin|block",
            Annotation{ 2, 17, Tag::BEGIN, 1, 19, 1, 0, 11, "@begin", "block" }.fieldValues()
        );
    }

    YW_TEST(Annotation, InsertingFirstAnnotationYieldsGeneratedId1)
    {
        Assert::AreEqual(1, ywdb.insert(Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }));
    }

    YW_TEST(Annotation, InsertingQualifyingAnnotationYieldsGeneratedId2)
    {
        row_id annotation1;
        Expect::AreEqual(1, (annotation1 = ywdb.insert(Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" })));
        Assert::AreEqual(2, ywdb.insert(Annotation{ auto_id, extraction17, Tag::IN, annotation1, line25, 1, 0, 7, "@in", "port" }));
    }

    YW_TEST(Annotation, SelectingFirstAnnotationByIdYieldsAssignedFieldValues) {

        Expect::AreEqual(1, ywdb.insert(Annotation{ auto_id,extraction17,  Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }));

        auto annotation = ywdb.selectAnnotationById(1);

        Assert::AreEqual(1L, annotation.id);
        Assert::AreEqual(Tag::BEGIN, annotation.tag);
        Assert::IsNull(annotation.qualifiesId);
        Assert::AreEqual(19, annotation.lineId);
        Assert::AreEqual(1, annotation.rankOnLine);
        Assert::AreEqual(0, annotation.start);
        Assert::AreEqual(11, annotation.end);
        Assert::AreEqual("@begin", annotation.keyword);
        Assert::AreEqual("block", annotation.value.getValue());
        Assert::AreEqual("1|17|BEGIN|NULL|19|1|0|11|@begin|block", annotation.fieldValues());
    }

    YW_TEST(Annotation, SelectingQualifyingAnnotationByIdYieldsAssignedFieldValues) {

        long annotation1;
        Expect::AreEqual(1, (annotation1 = ywdb.insert(Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" })));
        Expect::AreEqual(2, ywdb.insert(Annotation{ auto_id, extraction17, Tag::IN, annotation1, line25, 1, 0, 7, "@in", "port" }));

        auto annotation = ywdb.selectAnnotationById(2);

        Assert::AreEqual(2L, annotation.id);
        Assert::AreEqual(Tag::IN, annotation.tag);
        Assert::AreEqual(1L, annotation.qualifiesId);
        Assert::AreEqual(25, annotation.lineId);
        Assert::AreEqual(1, annotation.rankOnLine);
        Assert::AreEqual(0, annotation.start);
        Assert::AreEqual(7, annotation.end);
        Assert::AreEqual("@in", annotation.keyword);
        Assert::AreEqual("port", annotation.value.getValue());
        Assert::AreEqual("2|17|IN|1|25|1|0|7|@in|port", annotation.fieldValues());
    }

    YW_TEST(Annotation, SelectingNonexistent) {
        try {
            auto source = ywdb.selectAnnotationById(1);
            Assert::Fail();
        }
        catch (std::runtime_error& e) {
            Assert::AreEqual("No annotation row with that id", e.what());
        }
    }

    YW_TEST(Annotation, SelectTopLevelAnnotationsReturnsOneRowWhenJustOneBeginAnnotation) {
        Expect::AreEqual(1, (ywdb.insert(Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" })));

        auto selectedAnnotations = ywdb.selectTopLevelAnnotations();

        Assert::AreEqual(1, selectedAnnotations.size());
        Assert::AreEqual(Annotation{ 1, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }, selectedAnnotations[0]);
    }

    YW_TEST(Annotation, SelectTopLevelAnnotationsReturnsTwoRowsWhenJustTwoBeginAnnotations) {
        ywdb.insert(Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" });
        ywdb.insert(Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line119, 1, 0, 18, "@begin", "second block" });
        auto selectedAnnotations = ywdb.selectTopLevelAnnotations();

        Assert::AreEqual(2, selectedAnnotations.size());
        Assert::AreEqual(Annotation{ 1, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }, selectedAnnotations[0]);
        Assert::AreEqual(Annotation{ 2, extraction17, Tag::BEGIN, null_id, line119, 1, 0, 18, "@begin", "second block" }, selectedAnnotations[1]);
    }

    YW_TEST(Annotation, SelectTopLevelAnnotationsReturnsTwoRowsWhenTwoTreesInserted) {

        auto firstRoot = Annotation{ 1, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" };
        ywdb.insert(firstRoot);
        ywdb.insert(Annotation{ 2, extraction17, Tag::IN, firstRoot.id, line25, 1, 0, 8, "@in", "port" });
        ywdb.insert(Annotation{ 3, extraction17, Tag::END, firstRoot.id, line77, 1, 0, 10, "@end", "block" });

        auto secondRoot = Annotation{ 4, extraction17, Tag::BEGIN, null_id, line119, 1, 0, 18, "@begin", "second block" };
        ywdb.insert(secondRoot);
        ywdb.insert(Annotation{ auto_id, extraction17, Tag::IN, secondRoot.id, line125, 1, 0, 15, "@in", "second port" });
        ywdb.insert(Annotation{ auto_id, extraction17, Tag::END, secondRoot.id, line177, 1, 0, 17, "@end", "second block" });

        auto selectedAnnotations = ywdb.selectTopLevelAnnotations();

        Assert::AreEqual(2, selectedAnnotations.size());
        Assert::AreEqual(Annotation{ 1, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }, selectedAnnotations[0]);
        Assert::AreEqual(Annotation{ 4, extraction17, Tag::BEGIN, null_id, line119, 1, 0, 18, "@begin", "second block" }, selectedAnnotations[1]);
    }

    YW_TEST(Annotation, SelectAnnotationTreeReturnsOneRowWhenRootAnnotationHasNoChildren) {
        auto annotation1 = Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" };
        Expect::AreEqual("NULL|17|BEGIN|NULL|19|1|0|11|@begin|block", annotation1.fieldValues());

        row_id insertedAnnotationId;
        Expect::AreEqual(1, (insertedAnnotationId = ywdb.insert(annotation1)));
        Expect::AreEqual("1|17|BEGIN|NULL|19|1|0|11|@begin|block", annotation1.fieldValues());

        auto selectedAnnotations = ywdb.selectAnnotationTree(insertedAnnotationId);

        Assert::AreEqual(1, selectedAnnotations.size());
        Assert::AreEqual(Annotation{ 1, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }, selectedAnnotations[0]);
    }

    YW_TEST(Annotation, SelectAnnotationTreeReturnsAllRowWhenOnlyRootAndChildrenInserted) {
        auto annotation1 = Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" };
        row_id rootAnnotationId;
        Expect::AreEqual(1, (rootAnnotationId = ywdb.insert(annotation1)));
        Expect::AreEqual("1|17|BEGIN|NULL|19|1|0|11|@begin|block", annotation1.fieldValues());
        auto annotation2 = Annotation{ auto_id, extraction17, Tag::IN, rootAnnotationId, line25, 1, 0, 8, "@in", "port" };
        auto annotation3 = Annotation{ auto_id, extraction17, Tag::END, rootAnnotationId, line77, 1, 0, 10, "@end", "block" };
        ywdb.insert(annotation2);
        ywdb.insert(annotation3);
        Expect::AreEqual("2|17|IN|1|25|1|0|8|@in|port", annotation2.fieldValues());
        Expect::AreEqual("3|17|END|1|77|1|0|10|@end|block", annotation3.fieldValues());

        auto selectedAnnotations = ywdb.selectAnnotationTree(rootAnnotationId);

        Assert::AreEqual(3, selectedAnnotations.size());
        Assert::AreEqual(Annotation{ 1, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }, selectedAnnotations[0]);
        Assert::AreEqual(Annotation{ 2, extraction17, Tag::IN, 1, line25, 1, 0, 8, "@in", "port" }, selectedAnnotations[1]);
        Assert::AreEqual(Annotation{ 3, extraction17, Tag::END, 1, line77, 1, 0, 10, "@end", "block" }, selectedAnnotations[2]);
    }

    YW_TEST(Annotation, SelectAnnotationTreeReturnsTwoDifferentTreesDependingOnRoot) {

        auto annotation1 = Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" };
        row_id firstRootAnnotationId;
        Expect::AreEqual(1, (firstRootAnnotationId = ywdb.insert(annotation1)));
        auto annotation2 = Annotation{ auto_id, extraction17, Tag::IN, firstRootAnnotationId, line25, 1, 0, 8, "@in", "port" };
        auto annotation3 = Annotation{ auto_id, extraction17, Tag::END, firstRootAnnotationId, line77, 1, 0, 10, "@end", "block" };
        ywdb.insert(annotation2);
        ywdb.insert(annotation3);
        Expect::AreEqual(3, ywdb.getRowCount("annotation"));

        auto annotation4 = Annotation{ auto_id, extraction17, Tag::BEGIN, null_id, line119, 1, 0, 18, "@begin", "second block" };
        row_id secondRootAnnotationId;
        Expect::AreEqual(4, (secondRootAnnotationId = ywdb.insert(annotation4)));
        auto annotation5 = Annotation{ auto_id, extraction17, Tag::IN, secondRootAnnotationId, line125, 1, 0, 15, "@in", "second port" };
        auto annotation6 = Annotation{ auto_id, extraction17, Tag::END, secondRootAnnotationId, line177, 1, 0, 17, "@end", "second block" };
        ywdb.insert(annotation5);
        ywdb.insert(annotation6);
        Expect::AreEqual(6, ywdb.getRowCount("annotation"));

        auto firstAnnotationTree = ywdb.selectAnnotationTree(firstRootAnnotationId);
        auto secondAnnotationTree = ywdb.selectAnnotationTree(secondRootAnnotationId);
        Assert::AreEqual(3, firstAnnotationTree.size());
        Assert::AreEqual(Annotation{ 1, extraction17, Tag::BEGIN, null_id, line19, 1, 0, 11, "@begin", "block" }, firstAnnotationTree[0]);
        Assert::AreEqual(Annotation{ 2, extraction17, Tag::IN, 1, line25, 1, 0, 8, "@in", "port" }, firstAnnotationTree[1]);
        Assert::AreEqual(Annotation{ 3, extraction17, Tag::END, 1, line77, 1, 0, 10, "@end", "block" }, firstAnnotationTree[2]);

        Assert::AreEqual(3, secondAnnotationTree.size());
        Assert::AreEqual(Annotation{ 4, extraction17, Tag::BEGIN, null_id, line119, 1, 0, 18, "@begin", "second block" }, secondAnnotationTree[0]);
        Assert::AreEqual(Annotation{ 5, extraction17, Tag::IN, 4, line125, 1, 0, 15, "@in", "second port" }, secondAnnotationTree[1]);
        Assert::AreEqual(Annotation{ 6, extraction17, Tag::END, 4, line177, 1, 0, 17, "@end", "second block" }, secondAnnotationTree[2]);
    }

YW_TEST_END