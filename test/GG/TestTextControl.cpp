#include <GG/GUI.h>
#include <GG/Clr.h>
#include <GG/ClrConstants.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/StyleFactory.h>

#include "TestApp.h"

int main(int argc, char* argv[])
{

    TestApp app(argc, argv);

    const std::string message = "Are <i>we</i> Готово yet?"; // That Russian word means "Done", ha.
    const std::set<GG::UnicodeCharset> charsets_ = GG::UnicodeCharsetsToRender(message);
    const std::vector<GG::UnicodeCharset> charsets(charsets_.begin(), charsets_.end());

    const boost::shared_ptr<GG::Font> font =
    GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(12, &charsets[0], &charsets[0] + charsets.size());

    GG::Wnd* quit_dlg =
    new GG::ThreeButtonDlg(GG::X(10), GG::Y(10), message, font, GG::CLR_SHADOW,
                           GG::CLR_SHADOW, GG::CLR_SHADOW, GG::CLR_WHITE, 1);


    app.Run(quit_dlg);

    return 0;
}
