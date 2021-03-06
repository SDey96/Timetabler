#include "constraint_adder.h"

#include "clauses.h"
#include "constraint_encoder.h"
#include "core/SolverTypes.h"
#include "global.h"
#include "time_tabler.h"
#include "utils.h"
#include <iostream>
#include <vector>

using namespace Minisat;

/**
 * @brief      Constructs the ConstraintAdder object.
 *
 * @param      encoder     The encoder
 * @param      timeTabler  The time tabler
 */
ConstraintAdder::ConstraintAdder(ConstraintEncoder *encoder,
                                 TimeTabler *timeTabler) {
    this->encoder = encoder;
    this->timeTabler = timeTabler;
}

/**
 * @brief      Imposes the constraint that a given FieldType value should be
 * true for at most one Course at a time.
 *
 * For example, this includes constraints such as enforcing that a given
 * Instructor cannot have two courses at the same time. Here, a time refers to a
 * combination of segment and slot. This is not added directly, but called by
 * other functions.
 *
 * @param[in]  fieldType  The field type on which this constraint is imposed
 *
 * @return     A Clauses object describing this constraint
 */
Clauses ConstraintAdder::fieldSingleValueAtATime(FieldType fieldType) {
    Clauses result;
    result.clear();
    std::vector<Course> courses = timeTabler->data.courses;
    for (int i = 0; i < courses.size(); i++) {
        for (int j = i + 1; j < courses.size(); j++) {
            /*
             * For every pair of courses, either the field value of the
             * FieldType is different or their times do not intersect
             */
            Clauses antecedent =
                encoder->hasSameFieldTypeNotSameValue(i, j, fieldType);
            Clauses consequent = encoder->notIntersectingTime(i, j);
            result.addClauses(antecedent | consequent);
        }
    }
    return result;
}

/**
 * @brief      Imposes the constraint that an Instructor can have only a single
 *             course at a given time.
 *
 * This simply calls fieldSingleValueAtATime with the FieldType as
 * FieldType::instructor. By default, this constraint is hard.
 *
 * @return     A Clauses object describing this constraint
 */
Clauses ConstraintAdder::instructorSingleCourseAtATime() {
    return fieldSingleValueAtATime(FieldType::instructor);
}

/**
 * @brief      Imposes the constraint that a Classroom can have only a single
 *             course at a given time.
 *
 * This simply calls fieldSingleValueAtATime with the FieldType as
 * FieldType::classroom. By default, this constraint is hard.
 *
 * @return     A Clauses object describing this constraint
 */
Clauses ConstraintAdder::classroomSingleCourseAtATime() {
    return fieldSingleValueAtATime(FieldType::classroom);
}

/**
 * @brief      Imposes the constraint that if two courses are core for
 *             a Program, then they are not scheduled at an intersecting
 *             time.
 *
 * By default, this constraint is hard.
 *
 * @return     A Clauses object describing the constraint
 */
Clauses ConstraintAdder::programSingleCoreCourseAtATime() {
    Clauses result;
    result.clear();
    std::vector<Course> courses = timeTabler->data.courses;
    for (int i = 0; i < courses.size(); i++) {
        for (int j = i + 1; j < courses.size(); j++) {
            /*
             * For every pair of courses, either there is no Program for which
             * they are both core or their times do not intersect
             */
            Clauses antecedent = encoder->hasNoCommonCoreProgram(i, j);
            Clauses consequent = encoder->notIntersectingTime(i, j);
            result.addClauses(antecedent | consequent);
        }
    }
    return result;
}

/**
 * @brief      Imposes the constraint that a minor Course must be
 *             scheduled in a minor slot.
 *
 * By default, this constraint is hard.
 *
 * @return     A Clauses object describing the constraint
 */
Clauses ConstraintAdder::minorInMinorTime() {
    Clauses result;
    result.clear();
    std::vector<Course> courses = timeTabler->data.courses;
    for (int i = 0; i < courses.size(); i++) {
        /*
         * a minor course must be in a minor Slot.
         * a non-minor course must not be in a minor Slot.
         */
        Clauses antecedent = encoder->isMinorCourse(i);
        Clauses consequent = encoder->slotInMinorTime(i);
        result.addClauses(antecedent >> consequent);
        result.addClauses(consequent >> antecedent);
    }
    return result;
}

/**
 * @brief      Imposes the constraint that the given FieldType has exactly one
 * value True for a given Course.
 *
 * For example, it imposes the constraint that a given Course has exactly one
 * Instructor. By default, these constraints are hard. However, there exist high
 * level variables to notify the user if a particular constraint in this
 * category could not be satisfied, in order to help in making the necessary
 * modifications.
 *
 * @param[in]  fieldType  The field type on which this constraint is imposed
 *
 * @return     A Clauses object describing the constraint
 */
Clauses ConstraintAdder::exactlyOneFieldValuePerCourse(FieldType fieldType) {
    Clauses result;
    result.clear();
    std::vector<Course> courses = timeTabler->data.courses;
    for (int i = 0; i < courses.size(); i++) {
        // exactly one field value must be true
        Clauses exactlyOneFieldValue =
            encoder->hasExactlyOneFieldValueTrue(i, fieldType);
        Clauses cclause(timeTabler->data.highLevelVars[i][fieldType]);
        // high level variable implies the clause, and by default is hard
        // if high level variable is false, this clause could not be satisfied
        // this provides a reason to the user
        result.addClauses(cclause >> exactlyOneFieldValue);
    }
    return result;
}

/**
 * @brief      Adds all the constraints with their respective weights using the
 * TimeTabler object to the solver.
 */
void ConstraintAdder::addConstraints() {
    std::vector<int> weights = timeTabler->data.predefinedClausesWeights;

    // add the constraints to the formula
    timeTabler->addClauses(
        instructorSingleCourseAtATime(),
        weights[PredefinedClauses::instructorSingleCourseAtATime]);
    timeTabler->addClauses(
        classroomSingleCourseAtATime(),
        weights[PredefinedClauses::classroomSingleCourseAtATime]);
    timeTabler->addClauses(
        programSingleCoreCourseAtATime(),
        weights[PredefinedClauses::programSingleCoreCourseAtATime]);
    timeTabler->addClauses(minorInMinorTime(),
                           weights[PredefinedClauses::minorInMinorTime]);
    timeTabler->addClauses(
        programAtMostOneOfCoreOrElective(),
        weights[PredefinedClauses::programAtMostOneOfCoreOrElective]);

    timeTabler->addClauses(exactlyOneFieldValuePerCourse(FieldType::slot),
                           weights[PredefinedClauses::exactlyOneSlotPerCourse]);
    timeTabler->addClauses(
        exactlyOneFieldValuePerCourse(FieldType::classroom),
        weights[PredefinedClauses::exactlyOneClassroomPerCourse]);
    timeTabler->addClauses(
        exactlyOneFieldValuePerCourse(FieldType::instructor),
        weights[PredefinedClauses::exactlyOneInstructorPerCourse]);
    timeTabler->addClauses(
        exactlyOneFieldValuePerCourse(FieldType::isMinor),
        weights[PredefinedClauses::exactlyOneIsMinorPerCourse]);
    timeTabler->addClauses(
        exactlyOneFieldValuePerCourse(FieldType::segment),
        weights[PredefinedClauses::exactlyOneSegmentPerCourse]);

    timeTabler->addClauses(coreInMorningTime(),
                           weights[PredefinedClauses::coreInMorningTime]);
    timeTabler->addClauses(
        electiveInNonMorningTime(),
        weights[PredefinedClauses::electiveInNonMorningTime]);
}

/*Clauses ConstraintAdder::softConstraints() {
    return existingAssignmentClausesSoft();
}*/

/**
 * @brief      Imposes that a core course is given a morning slot.
 *
 * By default, this constraint is soft.
 *
 * @return     A Clauses object describing the constraint
 */
Clauses ConstraintAdder::coreInMorningTime() {
    Clauses result;
    result.clear();
    std::vector<Course> courses = timeTabler->data.courses;
    for (int i = 0; i < courses.size(); i++) {
        Clauses coreCourse = encoder->isCoreCourse(i);
        Clauses morningTime = encoder->courseInMorningTime(i);
        result.addClauses(coreCourse >> morningTime);
    }
    return result;
}

/**
 * @brief      Imposes that a elective course is given a afternoon slot.
 *
 * By default, this constraint is soft.
 *
 * @return     A Clauses object describing the constraint
 */
Clauses ConstraintAdder::electiveInNonMorningTime() {
    Clauses result;
    result.clear();
    std::vector<Course> courses = timeTabler->data.courses;
    for (int i = 0; i < courses.size(); i++) {
        Clauses coreCourse = encoder->isElectiveCourse(i);
        Clauses morningTime = encoder->courseInMorningTime(i);
        result.addClauses(coreCourse >> (~morningTime));
    }
    return result;
}

/**
 * @brief      Imposes the constraint that for a course, a program can either
 *             be a core program or an elective program, or neither, but not
 * both.
 *
 * By default, this constraint is hard.
 *
 * @return     A Clauses object describing the constraint
 */
Clauses ConstraintAdder::programAtMostOneOfCoreOrElective() {
    Clauses result;
    result.clear();
    std::vector<Course> courses = timeTabler->data.courses;
    for (int i = 0; i < courses.size(); i++) {
        result.addClauses(encoder->programAtMostOneOfCoreOrElective(i));
    }
    return result;
}