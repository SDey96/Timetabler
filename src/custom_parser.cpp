#include "custom_parser.h"
#include "clauses.h"
#include "global.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <tao/pegtl.hpp>
#include <vector>

namespace pegtl = tao::TAOCPP_PEGTL_NAMESPACE;

template <typename Rule> struct action : pegtl::nothing<Rule> {};

/**
 * @brief      Parse integer: Store the integer in the object
 */
struct integer
    : pegtl::seq<pegtl::opt<pegtl::one<'-'>>, pegtl::plus<pegtl::digit>> {};
template <> struct action<integer> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.integer = std::stoi(in.string());
    }
};

/**
 * @brief      Parse "IN"
 */
struct instr : TAOCPP_PEGTL_KEYWORD("IN") {};

/**
 * @brief      Parse "NOT": Store that NOT keyword is present in the custom constraint
 */
struct notstr : TAOCPP_PEGTL_KEYWORD("NOT") {};
template <> struct action<notstr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.isNot = true;
    }
};

/**
 * @brief      Parse "NOT"
 */
struct andstr : TAOCPP_PEGTL_KEYWORD("AND") {};

/**
 * @brief      Parse "OR"
 */
struct orstr : TAOCPP_PEGTL_KEYWORD("OR") {};

/**
 * @brief      Parse "CLASSROOM": Store the field type to be classroom
 * which will be used while forming the custom consraint
 */
struct classroomstr : TAOCPP_PEGTL_KEYWORD("CLASSROOM") {};
template <> struct action<classroomstr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.fieldType = FieldValuesType::CLASSROOM;
    }
};

/**
 * @brief      Parse "SLOT": Similar to classroom
 */
struct slotstr : TAOCPP_PEGTL_KEYWORD("SLOT") {};
template <> struct action<slotstr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.fieldType = FieldValuesType::SLOT;
    }
};

/**
 * @brief      Parse "COURSE": Similar to classroom
 */
struct coursestr : TAOCPP_PEGTL_KEYWORD("COURSE") {};
template <> struct action<coursestr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.fieldType = FieldValuesType::COURSE;
    }
};

/**
 * @brief      Parse "INSTRUCTOR": Similar to classroom
 */
struct instructorstr : TAOCPP_PEGTL_KEYWORD("INSTRUCTOR") {};
template <> struct action<instructorstr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.fieldType = FieldValuesType::INSTRUCTOR;
    }
};

/**
 * @brief      Parse "SEGMENT": Similar to classroom
 */
struct segmentstr : TAOCPP_PEGTL_KEYWORD("SEGMENT") {};
template <> struct action<segmentstr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.fieldType = FieldValuesType::SEGMENT;
    }
};

/**
 * @brief      Parse "ISMNOR": Similar to classroom
 */
struct isminorstr : TAOCPP_PEGTL_KEYWORD("ISMINOR") {};
template <> struct action<isminorstr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.fieldType = FieldValuesType::ISMINOR;
    }
};

/**
 * @brief      Parse "PROGRAM": Similar to classroom
 */
struct programstr : TAOCPP_PEGTL_KEYWORD("PROGRAM") {};
template <> struct action<programstr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.fieldType = FieldValuesType::PROGRAM;
    }
};

/**
 * @brief      Constraint is on one of the instructor, segment, isminor, program.
 * isNot, classSame, slotSame, classNotSame, slotNotSame are reset.
 */
struct fieldtype
    : pegtl::sor<instructorstr, segmentstr, isminorstr, programstr> {};
template <> struct action<fieldtype> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.isNot = false;
        obj.classSame = false;
        obj.slotSame = false;
        obj.classNotSame = false;
        obj.slotNotSame = false;
    }
};

/**
 * @brief      Parse a value of the field that is specified in the constraint
 */
struct value
    : pegtl::plus<pegtl::sor<pegtl::range<'a', 'z'>, pegtl::range<'A', 'Z'>,
                             pegtl::digit, pegtl::one<'.'>, pegtl::one<'-'>,
                             pegtl::one<'@'>, pegtl::space>> {};
template <> struct action<value> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        std::string val = in.string();
        bool found = false;
        if (obj.fieldType == FieldValuesType::INSTRUCTOR) {
            for (int i = 0; i < obj.timeTabler->data.instructors.size(); i++) {
                if (obj.timeTabler->data.instructors[i].getName() == val) {
                    found = true;
                    obj.instructorValues.push_back(i);
                    break;
                }
            }
            if (!found) {
                std::cout << "Instructor " << val << " does not exist."
                          << std::endl;
                exit(1);
            }
            found = false;
        } else if (obj.fieldType == FieldValuesType::COURSE) {
            for (int i = 0; i < obj.timeTabler->data.courses.size(); i++) {
                std::cout << obj.timeTabler->data.courses[i].getName()
                          << std::endl;
                if (obj.timeTabler->data.courses[i].getName() == val) {
                    found = true;
                    obj.courseValues.push_back(i);
                    break;
                }
            }
            if (!found) {
                std::cout << "Course " << val << " does not exist."
                          << std::endl;
                exit(1);
            }
            found = false;
        } else if (obj.fieldType == FieldValuesType::SEGMENT) {
            for (int i = 0; i < obj.timeTabler->data.segments.size(); i++) {
                if (obj.timeTabler->data.segments[i].getName() == val) {
                    found = true;
                    obj.segmentValues.push_back(i);
                    break;
                }
            }
            if (!found) {
                std::cout << "Segment " << val << " does not exist."
                          << std::endl;
                exit(1);
            }
            found = false;
        } else if (obj.fieldType == FieldValuesType::PROGRAM) {
            for (int i = 0; i < obj.timeTabler->data.programs.size(); i++) {
                if (obj.timeTabler->data.programs[i].getNameWithType() == val) {
                    found = true;
                    obj.programValues.push_back(i);
                    break;
                }
            }
            if (!found) {
                std::cout << "Program " << val << " does not exist."
                          << std::endl;
                exit(1);
            }
            found = false;
        } else if (obj.fieldType == FieldValuesType::ISMINOR) {
            for (int i = 0; i < obj.timeTabler->data.isMinors.size(); i++) {
                if (obj.timeTabler->data.isMinors[i].getName() == val) {
                    found = true;
                    obj.isMinorValues.push_back(i);
                    break;
                }
            }
            if (!found) {
                std::cout << "IsMinor " << val << " does not exist."
                          << std::endl;
                exit(1);
            }
            found = false;
        } else if (obj.fieldType == FieldValuesType::CLASSROOM) {
            for (int i = 0; i < obj.timeTabler->data.classrooms.size(); i++) {
                if (obj.timeTabler->data.classrooms[i].getName() == val) {
                    found = true;
                    obj.classValues.push_back(i);
                    break;
                }
            }
            if (!found) {
                std::cout << "Classroom " << val << " does not exist."
                          << std::endl;
                exit(1);
            }
            found = false;
        } else if (obj.fieldType == FieldValuesType::SLOT) {
            for (int i = 0; i < obj.timeTabler->data.slots.size(); i++) {
                if (obj.timeTabler->data.slots[i].getName() == val) {
                    found = true;
                    obj.slotValues.push_back(i);
                    break;
                }
            }
            if (!found) {
                std::cout << "Slot " << val << " does not exist." << std::endl;
                exit(1);
            }
            found = false;
        }
    }
};

/**
 * @brief      Parse * as all values of the specified field
 */
struct allvalues : pegtl::pad<pegtl::one<'*'>, pegtl::space> {};
template <> struct action<allvalues> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        std::string val = in.string();
        if (obj.fieldType == FieldValuesType::INSTRUCTOR) {
            for (int i = 0; i < obj.timeTabler->data.instructors.size(); i++) {
                obj.instructorValues.push_back(i);
            }
        } else if (obj.fieldType == FieldValuesType::COURSE) {
            for (int i = 0; i < obj.timeTabler->data.courses.size(); i++) {
                obj.courseValues.push_back(i);
            }
        } else if (obj.fieldType == FieldValuesType::SEGMENT) {
            for (int i = 0; i < obj.timeTabler->data.segments.size(); i++) {
                obj.segmentValues.push_back(i);
            }
        } else if (obj.fieldType == FieldValuesType::PROGRAM) {
            for (int i = 0; i < obj.timeTabler->data.programs.size(); i++) {
                obj.programValues.push_back(i);
            }
        } else if (obj.fieldType == FieldValuesType::ISMINOR) {
            for (int i = 0; i < obj.timeTabler->data.isMinors.size(); i++) {
                obj.isMinorValues.push_back(i);
            }
        } else if (obj.fieldType == FieldValuesType::CLASSROOM) {
            for (int i = 0; i < obj.timeTabler->data.classrooms.size(); i++) {
                obj.classValues.push_back(i);
            }
        } else if (obj.fieldType == FieldValuesType::SLOT) {
            for (int i = 0; i < obj.timeTabler->data.slots.size(); i++) {
                obj.slotValues.push_back(i);
            }
        }
    }
};

/**
 * @brief      Parse "SAME": Used to specify constraints on courses with same filed values
 */
struct sameval : pegtl::pad<TAOCPP_PEGTL_KEYWORD("SAME"), pegtl::space> {};
template <> struct action<sameval> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        if (obj.fieldType == FieldValuesType::CLASSROOM) {
            obj.classSame = true;
        } else if (obj.fieldType == FieldValuesType::SLOT) {
            obj.slotSame = true;
        }
    }
};

/**
 * @brief      Parse "NOTSAME": TODO
 */
struct notsameval : pegtl::pad<TAOCPP_PEGTL_KEYWORD("NOTSAME"), pegtl::space> {};
template <> struct action<notsameval> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        if (obj.fieldType == FieldValuesType::CLASSROOM) {
            obj.classNotSame = true;
        } else if (obj.fieldType == FieldValuesType::SLOT) {
            obj.slotNotSame = true;
        }
    }
};

/**
 * @brief      Parse list of values of a field specified in the constraint
 */
struct listvalues
    : pegtl::seq<pegtl::pad<pegtl::one<'{'>, pegtl::space>,
                 pegtl::list<value, pegtl::one<','>, pegtl::space>,
                 pegtl::pad<pegtl::one<'}'>, pegtl::space>> {};

/**
 * @brief      Parse values
 */
struct values : pegtl::sor<allvalues, listvalues, sameval, notsameval> {};

/**
 * @brief      Parse classrooms
 */
struct classroomdecl
    : pegtl::seq<pegtl::pad<classroomstr, pegtl::space>, values> {};

/**
 * @brief      Parse slots
 */
struct slotdecl : pegtl::seq<pegtl::pad<slotstr, pegtl::space>, values> {};

/**
 * @brief      Parse courses
 */
struct coursedecl : pegtl::seq<pegtl::pad<coursestr, pegtl::space>, values> {};

/**
 * @brief      Parse single decl in consequent of the constraint
 */
struct decl : pegtl::sor<slotdecl, classroomdecl> {};

/**
 * @brief      Parse multi decls in consequent of the constraint
 */
struct decls : pegtl::list<decl, andstr, pegtl::space> {};

/**
 * @brief      Parse fieldd decl in antecedent of the constraint
 */
struct fielddecl : pegtl::seq<pegtl::pad<fieldtype, pegtl::space>, values> {};

/**
 * @brief      Parse multi field decls in antecedent of the constraint
 */
struct fielddecls : pegtl::opt<pegtl::list<fielddecl, andstr, pegtl::space>> {};

/**
 * @brief      Makes an antecedent.
 *
 * @param      obj     The object
 * @param[in]  course  The course
 *
 * @return     Clauses corresponding to the antecedent
 */
Clauses makeAntecedent(Object &obj, int course) {
    Clauses ante, clause;
    if (obj.instructorValues.size() > 0) {
        clause = obj.constraintEncoder->hasFieldTypeListedValues(
            course, FieldType::instructor, obj.instructorValues);
        ante = ante & clause;
    }
    if (obj.programValues.size() > 0) {
        clause = obj.constraintEncoder->hasFieldTypeListedValues(
            course, FieldType::program, obj.programValues);
        ante = ante & clause;
    }
    if (obj.segmentValues.size() > 0) {
        clause = obj.constraintEncoder->hasFieldTypeListedValues(
            course, FieldType::segment, obj.segmentValues);
        ante = ante & clause;
    }
    if (obj.isMinorValues.size() > 0) {
        clause = obj.constraintEncoder->hasFieldTypeListedValues(
            course, FieldType::isMinor, obj.isMinorValues);
        ante = ante & clause;
    }
    return ante;
}

/**
 * @brief      Makes a consequent.
 *
 * @param      obj     The object
 * @param[in]  course  The course
 * @param[in]  i       Index of course in courseValues
 *
 * @return     Clauses corresponding to the consequent
 */
Clauses makeConsequent(Object &obj, int course, int i) {
    Clauses cons, clause;
    if (obj.classSame) {
        for (int j = i + 1; j < obj.courseValues.size(); j++) {
            Clauses a = makeAntecedent(obj, obj.courseValues[j]);
            Clauses b = obj.constraintEncoder->hasSameFieldTypeAndValue(
                course, obj.courseValues[j], FieldType::classroom);
            a = a >> b;
            cons = cons & a;
        }
    }
    if (obj.classNotSame) {
        for (int j = i + 1; j < obj.courseValues.size(); j++) {
            Clauses a = makeAntecedent(obj, obj.courseValues[j]);
            Clauses b = obj.constraintEncoder->hasSameFieldTypeAndValue(
                course, obj.courseValues[j], FieldType::classroom);
            a = a >> (~b);
            cons = cons & a;
        }
    }
    if (obj.slotSame) {
        for (int j = i + 1; j < obj.courseValues.size(); j++) {
            Clauses a = makeAntecedent(obj, obj.courseValues[j]);
            Clauses b = obj.constraintEncoder->hasSameFieldTypeAndValue(
                course, obj.courseValues[j], FieldType::slot);
            a = a >> b;
            cons = cons & a;
        }
    }
    if (obj.slotNotSame) {
        for (int j = i + 1; j < obj.courseValues.size(); j++) {
            Clauses a = makeAntecedent(obj, obj.courseValues[j]);
            Clauses b = obj.constraintEncoder->hasSameFieldTypeAndValue(
                course, obj.courseValues[j], FieldType::slot);
            a = a >> (~b);
            cons = cons & a;
        }
    }
    if (obj.classValues.size() > 0) {
        clause = obj.constraintEncoder->hasFieldTypeListedValues(
            course, FieldType::classroom, obj.classValues);
        cons = cons & clause;
    }
    if (obj.slotValues.size() > 0) {
        clause = obj.constraintEncoder->hasFieldTypeListedValues(
            course, FieldType::slot, obj.slotValues);
        cons = cons & clause;
    }
    return cons;
}

/**
 * @brief      Parse a constraint
 */
struct constraint_expr : pegtl::seq<coursedecl, fielddecls, pegtl::opt<notstr>,
                                    pegtl::pad<instr, pegtl::space>, decls> {};
template <> struct action<constraint_expr> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        Clauses clauses;
        for (int i = 0; i < obj.courseValues.size(); i++) {
            int course = obj.courseValues[i];
            Clauses ante, cons, clause;
            ante = makeAntecedent(obj, course);
            cons = makeConsequent(obj, course, i);
            if (obj.isNot) {
                cons = ~cons;
            }
            clause = ante >> cons;
            clauses = clauses & clause;
        }
        obj.constraint = clauses;
        obj.courseValues.clear();
        obj.instructorValues.clear();
        obj.isMinorValues.clear();
        obj.programValues.clear();
        obj.segmentValues.clear();
    }
};

struct constraint_or;

/**
 * @brief      Parse constraint enclosed in braces
 */
struct constraint_braced
    : pegtl::seq<pegtl::if_must<pegtl::pad<pegtl::one<'('>, pegtl::space>,
                                constraint_or,
                                pegtl::pad<pegtl::one<')'>, pegtl::space>>> {};

/**
 * @brief      Parse negation of a constraint
 */
struct constraint_not
    : pegtl::seq<pegtl::pad<notstr, pegtl::space>, constraint_braced> {};
template <> struct action<constraint_not> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        Clauses clauses = obj.constraint;
        obj.constraint = ~clauses;
    }
};

/**
 * @brief      Parse a constraint: Constraint expression or negation of some
 * constraint expression or a constraint enclosed in parantheses
 */
struct constraint_val
    : pegtl::sor<constraint_expr, constraint_not, constraint_braced> {};
template <> struct action<constraint_val> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.constraintVals.push_back(obj.constraint);
    }
};

/**
 * @brief      Parse conjunction of constraints
 * Add all the constraints to obj.constraintAdds
 */
struct constraint_and : pegtl::list<constraint_val, andstr, pegtl::space> {};
template <> struct action<constraint_and> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        Clauses clauses = obj.constraintVals[0];
        for (unsigned i = 1; i < obj.constraintVals.size(); i++) {
            clauses = clauses & obj.constraintVals[i];
        }
        obj.constraintVals.clear();
        obj.constraintAnds.push_back(clauses);
    }
};

/**
 * @brief      Parse disjunction of constraints
 * The combined clauses for all the constraints are stored in obj.constraint
 */
struct constraint_or : pegtl::list<constraint_and, orstr, pegtl::space> {};
template <> struct action<constraint_or> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        Clauses clauses = obj.constraintAnds[0];
        for (unsigned i = 1; i < obj.constraintAnds.size(); i++) {
            clauses = clauses | obj.constraintAnds[i];
        }
        obj.constraintAnds.clear();
        obj.constraint = clauses;
    }
};

/**
 * @brief      Parse weighted constraint
 * Add the clauses corresponding to the constraint along with its weight
 */
struct wconstraint
    : pegtl::seq<pegtl::pad<constraint_or, pegtl::space>,
                 pegtl::pad<TAOCPP_PEGTL_KEYWORD("WEIGHT"), pegtl::space>,
                 pegtl::pad<integer, pegtl::space>> {};
template <> struct action<wconstraint> {
    template <typename Input> static void apply(const Input &in, Object &obj) {
        obj.timeTabler->addClauses(obj.constraint, obj.integer);
    }
};

/**
 * @brief      Parse constraints from the file, generate error on failure
 */
struct grammar
    : pegtl::try_catch<pegtl::must<pegtl::star<wconstraint>, pegtl::eof>> {};

template <typename Rule> struct control : pegtl::normal<Rule> {
    template <typename Input, typename... States>
    static void raise(const Input &in, States &&...) {
        std::cout << in.position() << " Error parsing custom constraints"
                  << std::endl;
        exit(1);
    }
};

void parseCustomConstraints(std::string file,
                            ConstraintEncoder *constraintEncoder,
                            TimeTabler *timeTabler) {
    Object obj;
    obj.constraintEncoder = constraintEncoder;
    obj.timeTabler = timeTabler;
    pegtl::file_input<> in(file);
    pegtl::parse<grammar, action, control>(in, obj);
}