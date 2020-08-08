
#include "Header.h"
#include "SaveWhiteSpaceDelimited.h"
#include "Error.h"
#include "SimpleDOM.h"
#include <ostream>

using std::string;

static void
Save_Recursive(cSimpleDOM& toSave, const std::string& indent, std::ostream& os)
{
    os << indent;
    os << '>' << toSave._name;
    std::vector<string> attributes;
    toSave.getAllAttributes(attributes);
    tSigned32 i;
    for(i = 0; i != SizeL(attributes) / 2; ++i)
    {
        os << " ." << attributes[i * 2] << ' ' << attributes[i * 2 + 1];
    }
    if(toSave._children.empty())
    {
        os << " <\n";
    }
    else
    {
        os << '\n';
        std::string insideIndent(indent);
        insideIndent.append("  ");
        for(i = 0; i != SizeL(toSave._children); ++i)
        {
            Save_Recursive(toSave._children[i], insideIndent, os);
        }
        os << indent << "<\n";
    }
}

void
SaveWhiteSpaceDelimited(cSimpleDOM& toSave, std::ostream& os)
{
    string indent;
    Save_Recursive(toSave, indent, os);
}
