add_definitions(-DNDEBUG -DFREEORION_BUILD_PARSE)

set (freeorionparse_HEADER
    CommonParams.h
    ConditionParserImpl.h
    Double.h
    EffectParser.h
    EffectParserImpl.h
    EnumParser.h
    Int.h
    Label.h
    Lexer.h
    Parse.h
    ParseImpl.h
    ReportParseError.h
    Tokens.h
    ValueRefParser.h
    ValueRefParserImpl.h
)

set (freeorionparse_SOURCE
    AlignmentsParser.cpp
    BuildingsParser.cpp
    CommonParamsParser.cpp
    ConditionParser1.cpp
    ConditionParser2.cpp
    ConditionParser3.cpp
    ConditionParser4.cpp
    ConditionParser5.cpp
    ConditionParser6.cpp
    ConditionParser7.cpp
    ConditionParser.cpp
    Double.cpp
    DoubleComplexValueRefParser.cpp
    DoubleValueRefParser.cpp
    EffectParser1.cpp
    EffectParser2.cpp
    EffectParser3.cpp
    EffectParser4.cpp
    EffectParser5.cpp
    EffectParser.cpp
    EmpireStatsParser.cpp
    EncyclopediaParser.cpp
    EnumParser.cpp
    FieldsParser.cpp
    FleetPlansParser.cpp
    Int.cpp
    IntComplexValueRefParser.cpp
    IntValueRefParser.cpp
    ItemsParser.cpp
    KeymapParser.cpp
    Label.cpp
    Lexer.cpp
    MonsterFleetPlansParser.cpp
    Parse.cpp
    PlanetEnvironmentValueRefParser.cpp
    PlanetSizeValueRefParser.cpp
    PlanetTypeValueRefParser.cpp
    ReportParseError.cpp
    ShipDesignsParser.cpp
    ShipHullsParser.cpp
    ShipPartsParser.cpp
    SpecialsParser.cpp
    SpeciesParser.cpp
    StarTypeValueRefParser.cpp
    StringComplexValueRefParser.cpp
    StringValueRefParser.cpp
    TechsParser.cpp
    Tokens.cpp
    UniverseObjectTypeValueRefParser.cpp
    ValueRefParserImpl.cpp
)

add_library(freeorionparse
    ${freeorionparse_HEADER}
    ${freeorionparse_SOURCE}
)

target_compile_options(
    freeorionparse PRIVATE
    $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
    $<$<AND:$<NOT:$<BOOL:${BUILD_TESTS}>>,$<CXX_COMPILER_ID:gnu>>:-O3>
    $<$<CXX_COMPILER_ID:clang>:-ftemplate-depth=512>
)

install(
    TARGETS freeorionparse
    LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)
