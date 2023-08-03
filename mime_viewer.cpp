#include "mime_viewer.hpp"

#include <ranges>
#include "ext_infrastructure/ext_thread.hpp"
#include "ext_infrastructure/ext_debug.hpp"

#include <QStringDecoder>
#include "ext_core/ext_mimedata.hpp"
#include "ext_core/ext_thread.hpp"
#include "ext_core/ext_string.hpp"

#include <QGuiApplication>
#include <QClipboard>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QStackedLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include "ext_widgets/flow_layout.hpp"
#include "ext_widgets/ext_text_edit.hpp"
#include "ext_widgets/scroll_area_vertical.hpp"

#include "ext_web/ext_webengine.hpp"

QWidget *create_mime_viewer_widget()
{
    QWidget *clipboard_history_widget_inner = new QWidget();
    QVBoxLayout *clipboard_history_vlayout = new QVBoxLayout(clipboard_history_widget_inner);
    QObject::connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, clipboard_history_widget_inner, [clipboard_history_vlayout]()
        {
            if(QMimeData const*mimeData = QGuiApplication::clipboard()->mimeData();mimeData!=nullptr)
            {
                {
                    std::shared_ptr<QMimeData>mimeData_copy(copyMimeData(mimeData));
                    if(QStringList formats=mimeData_copy-> formats();!formats.empty())
                    {
                        auto create_plain_text_widget=[](QString text)->PlainTextEdit*{
                            //                            QLabel*label=new QLabel(text);
                            //                            label->setFrameShape(QFrame::Shape::WinPanel);
                            //                            label->setFrameShadow(QFrame::Shadow::Sunken);
                            //                            label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse|Qt::TextInteractionFlag::TextSelectableByKeyboard|Qt::TextInteractionFlag::LinksAccessibleByMouse|Qt::TextInteractionFlag::LinksAccessibleByKeyboard);
                            //                            label->setWordWrap(true);
                            //                            return label;

                            PlainTextEdit*text_edit=new PlainTextEdit(std::forward_as_tuple(text));
                            make_plain_text_edit_readonly(text_edit);
                            //                            text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
                            //                            text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                            return text_edit;
                        };
                        QVBoxLayout*formats_vlayout=new QVBoxLayout();
                        clipboard_history_vlayout->insertLayout(0,formats_vlayout);
                        FlowLayout*formats_flow_layout=new FlowLayout();
                        formats_vlayout->addLayout(formats_flow_layout);
                        QStackedLayout*formats_stacked_layout=new QStackedLayout();
                        formats_stacked_layout->setContentsMargins(QMargins());
                        formats_vlayout->addLayout(formats_stacked_layout);

                        QButtonGroup*formats_button_group=new QButtonGroup(formats_vlayout);
                        formats_button_group->setExclusive(true);
                        QPushButton*copy_button=new QPushButton("copy");
                        formats_flow_layout->addWidget(copy_button);
                        int button_id=0;
                        for(QString format:formats)
                        {
                            QWidget*widget=new QWidget();
                            QFormLayout*form_layout=new QFormLayout(widget);
                            if(format==R"(text/uri-list)")
                            {
                                QWidget*urls_widget=new QWidget();
                                QFormLayout*urls_form_layout=new QFormLayout(urls_widget);urls_form_layout->setContentsMargins(QMargins());
                                {
                                    decltype(mimeData_copy->urls())::size_type i=0;
                                    for(QUrl url:mimeData_copy->urls())
                                    {
                                        urls_form_layout->addRow(QString::number(i),create_plain_text_widget(url.toDisplayString(QUrl::UrlFormattingOption::None)));
                                        ++i;
                                    }
                                }
                                form_layout->addRow("urls",urls_widget);
                            }
                            else if(format==R"(text/html)")
                            {
                                {
                                    if(true)
                                    {
                                        QTextBrowser*view_text_browser=new QTextBrowser();
                                        view_text_browser->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
                                        view_text_browser->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                        view_text_browser->setHtml(mimeData_copy->html());
                                        view_text_browser->setOpenExternalLinks(true);
                                        form_layout->addRow("html view",view_text_browser);
                                    }
                                    else
                                    {
                                        WebEngineView*view_web_view=new WebEngineView();
                                        {
                                            QTextLayout layout("", view_web_view->font());
                                            layout.beginLayout();
                                            QTextLine line = layout.createLine();
                                            layout.endLayout();
                                            line.setLeadingIncluded(true);
                                            view_web_view->setMinimumHeight(line.height() * 10);
                                        }
                                        view_web_view->setHtml(mimeData_copy->html());
                                        form_layout->addRow("html view",view_web_view);
                                    }
                                }
                                {
                                    form_layout->addRow("html source",create_plain_text_widget(mimeData_copy->html()));
                                }
                            }
                            else if(format==R"(text/plain;charset=utf-8)"||format==R"(text/plain)")
                            {
                                form_layout->addRow("text",create_plain_text_widget(mimeData_copy->text()));
                            }
                            else if(format==R"(application/x-color)")
                            {
                                form_layout->addRow("color",create_plain_text_widget(qvariant_cast<QColor>(mimeData_copy->colorData()).name(QColor::NameFormat::HexArgb)));
                            }
                            else if(format==R"(application/x-qt-image)")
                            {
                                QLabel*label=new QLabel();
                                label->setFrameShape(QFrame::Shape::WinPanel);
                                label->setFrameShadow(QFrame::Shadow::Raised);
                                label->setPixmap(QPixmap::fromImage(qvariant_cast<QImage>(mimeData_copy->imageData())));

                                QScrollArea*scroll_area=new QScrollArea();
                                scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                scroll_area->setWidget(label);

                                form_layout->addRow("image",scroll_area);
                            }
                            else
                            {
                            }
                            std::map<QStringConverter::Encoding,std::tuple<QStringDecoder,std::unique_ptr<QPointer<QLabel>>,std::unique_ptr<QPointer<PlainTextEdit>>>>encodings_states;
                            std::tuple<std::unique_ptr<QPointer<QLabel>>,std::unique_ptr<QPointer<PlainTextEdit>>>to_base64_states,from_base64_states,from_base64_strict_states,to_base64url_states,from_base64url_states,from_base64url_strict_states,to_percent_states,from_percent_states;
                            for(QStringConverter::Encoding encoding:{
                                    QStringConverter::Encoding::System,
                                    QStringConverter::Encoding::Latin1,
                                    QStringConverter::Encoding::Utf8,
                                    QStringConverter::Encoding::Utf16LE,
                                    QStringConverter::Encoding::Utf16BE,
                                    QStringConverter::Encoding::Utf32LE,
                                    QStringConverter::Encoding::Utf32BE,
                                })
                            {
                                QStringDecoder decoder(encoding,QStringConverter::Flag::ConvertInitialBom|QStringConverter::Flag::ConvertInitialBom);
                                QLabel*label=new QLabel(decoder.name());label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                encodings_states.emplace(std::piecewise_construct,std::forward_as_tuple(encoding),std::forward_as_tuple(std::move(decoder),std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit)));
                            }
                            {
                                QLabel*label=new QLabel("to_base64");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                to_base64_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            {
                                QLabel*label=new QLabel("from_base64");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                from_base64_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            {
                                QLabel*label=new QLabel("from_base64\nstrict");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                from_base64_strict_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            {
                                QLabel*label=new QLabel("to_base64url");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                to_base64url_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            {
                                QLabel*label=new QLabel("from_base64url");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                from_base64url_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            {
                                QLabel*label=new QLabel("from_base64url\nstrict");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                from_base64url_strict_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            {
                                QLabel*label=new QLabel("to_percent");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                to_percent_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            {
                                QLabel*label=new QLabel("from_percent");label->setEnabled(false);
                                PlainTextEdit*text_edit=create_plain_text_widget("");
                                text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
                                text_edit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
                                text_edit->setEnabled(false);
                                form_layout->addRow(label,text_edit);
                                from_percent_states=std::forward_as_tuple(std::make_unique<QPointer<QLabel>>(label),std::make_unique<QPointer<PlainTextEdit>>(text_edit));
                            }
                            [](auto mimeData_copy_data,auto encodings_states,auto to_base64_states,auto from_base64_states,auto from_base64_strict_states,auto to_base64url_states,auto from_base64url_states,auto from_base64url_strict_states,auto to_percent_states,auto from_percent_states)->future_t<void>
                            {
                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    QString result=QString::fromLatin1(mimeData_copy_data.toBase64(QByteArray::Base64Option::Base64Encoding|QByteArray::Base64Option::KeepTrailingEquals));
                                    co_await coroutine_dispatch_to_main_thread_t{};
                                    if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(to_base64_states),*std::get<1>(to_base64_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                    {
                                        qptext_edit->setPlainText(result);
                                        qplabel->setEnabled(true);
                                        qptext_edit->setEnabled(true);
                                    }
                                }
                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    if(QByteArray::FromBase64Result result_bytearray=QByteArray::fromBase64Encoding(mimeData_copy_data,QByteArray::Base64Option::Base64Encoding|QByteArray::Base64Option::IgnoreBase64DecodingErrors);result_bytearray.decodingStatus==QByteArray::Base64DecodingStatus::Ok)
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64_states),*std::get<1>(from_base64_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setPlainText(result);
                                            qplabel->setEnabled(true);
                                            qptext_edit->setEnabled(true);
                                        }
                                    }
                                    else
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        std::map<QByteArray::Base64DecodingStatus,QString>const error_messages{
                                            {QByteArray::Base64DecodingStatus::IllegalInputLength,"IllegalInputLength"},
                                            {QByteArray::Base64DecodingStatus::IllegalCharacter,"IllegalCharacter"},
                                            {QByteArray::Base64DecodingStatus::IllegalPadding,"IllegalPadding"},
                                            };
                                        result="decode error: "+error_messages.at(result_bytearray.decodingStatus)+'\n'+result;
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64_states),*std::get<1>(from_base64_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setEnabled(true);
                                            qptext_edit->setPlainText(result);
                                        }
                                    }
                                }
                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    if(QByteArray::FromBase64Result result_bytearray=QByteArray::fromBase64Encoding(mimeData_copy_data,QByteArray::Base64Option::Base64Encoding|QByteArray::Base64Option::AbortOnBase64DecodingErrors);result_bytearray.decodingStatus==QByteArray::Base64DecodingStatus::Ok)
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64_strict_states),*std::get<1>(from_base64_strict_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setPlainText(result);
                                            qplabel->setEnabled(true);
                                            qptext_edit->setEnabled(true);
                                        }
                                    }
                                    else
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        std::map<QByteArray::Base64DecodingStatus,QString>const error_messages{
                                            {QByteArray::Base64DecodingStatus::IllegalInputLength,"IllegalInputLength"},
                                            {QByteArray::Base64DecodingStatus::IllegalCharacter,"IllegalCharacter"},
                                            {QByteArray::Base64DecodingStatus::IllegalPadding,"IllegalPadding"},
                                            };
                                        result="decode error: "+error_messages.at(result_bytearray.decodingStatus)+'\n'+result;
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64_strict_states),*std::get<1>(from_base64_strict_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setEnabled(true);
                                            qptext_edit->setPlainText(result);
                                        }
                                    }
                                }

                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    QString result=QString::fromLatin1(mimeData_copy_data.toBase64(QByteArray::Base64Option::Base64UrlEncoding|QByteArray::Base64Option::KeepTrailingEquals));
                                    co_await coroutine_dispatch_to_main_thread_t{};
                                    if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(to_base64url_states),*std::get<1>(to_base64url_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                    {
                                        qptext_edit->setPlainText(result);
                                        qplabel->setEnabled(true);
                                        qptext_edit->setEnabled(true);
                                    }
                                }
                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    if(QByteArray::FromBase64Result result_bytearray=QByteArray::fromBase64Encoding(mimeData_copy_data,QByteArray::Base64Option::Base64UrlEncoding|QByteArray::Base64Option::IgnoreBase64DecodingErrors);result_bytearray.decodingStatus==QByteArray::Base64DecodingStatus::Ok)
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64url_states),*std::get<1>(from_base64url_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setPlainText(result);
                                            qplabel->setEnabled(true);
                                            qptext_edit->setEnabled(true);
                                        }
                                    }
                                    else
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        std::map<QByteArray::Base64DecodingStatus,QString>const error_messages{
                                            {QByteArray::Base64DecodingStatus::IllegalInputLength,"IllegalInputLength"},
                                            {QByteArray::Base64DecodingStatus::IllegalCharacter,"IllegalCharacter"},
                                            {QByteArray::Base64DecodingStatus::IllegalPadding,"IllegalPadding"},
                                            };
                                        result="decode error: "+error_messages.at(result_bytearray.decodingStatus)+'\n'+result;
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64url_states),*std::get<1>(from_base64url_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setEnabled(true);
                                            qptext_edit->setPlainText(result);
                                        }
                                    }
                                }
                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    if(QByteArray::FromBase64Result result_bytearray=QByteArray::fromBase64Encoding(mimeData_copy_data,QByteArray::Base64Option::Base64UrlEncoding|QByteArray::Base64Option::AbortOnBase64DecodingErrors);result_bytearray.decodingStatus==QByteArray::Base64DecodingStatus::Ok)
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64url_strict_states),*std::get<1>(from_base64url_strict_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setPlainText(result);
                                            qplabel->setEnabled(true);
                                            qptext_edit->setEnabled(true);
                                        }
                                    }
                                    else
                                    {
                                        QString result=QString::fromLatin1(result_bytearray.decoded);
                                        std::map<QByteArray::Base64DecodingStatus,QString>const error_messages{
                                            {QByteArray::Base64DecodingStatus::IllegalInputLength,"IllegalInputLength"},
                                            {QByteArray::Base64DecodingStatus::IllegalCharacter,"IllegalCharacter"},
                                            {QByteArray::Base64DecodingStatus::IllegalPadding,"IllegalPadding"},
                                            };
                                        result="decode error: "+error_messages.at(result_bytearray.decodingStatus)+'\n'+result;
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_base64url_strict_states),*std::get<1>(from_base64url_strict_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setEnabled(true);
                                            qptext_edit->setPlainText(result);
                                        }
                                    }
                                }

                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    QString result=QString::fromLatin1(mimeData_copy_data.toPercentEncoding());
                                    co_await coroutine_dispatch_to_main_thread_t{};
                                    if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(to_percent_states),*std::get<1>(to_percent_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                    {
                                        qptext_edit->setPlainText(result);
                                        qplabel->setEnabled(true);
                                        qptext_edit->setEnabled(true);
                                    }
                                }
                                {
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    QString result=QString::fromLatin1(QByteArray::fromPercentEncoding(mimeData_copy_data));
                                    co_await coroutine_dispatch_to_main_thread_t{};
                                    if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<0>(from_percent_states),*std::get<1>(from_percent_states));!qplabel.isNull()&&!qptext_edit.isNull())
                                    {
                                        qptext_edit->setPlainText(result);
                                        qplabel->setEnabled(true);
                                        qptext_edit->setEnabled(true);
                                    }
                                }

                                for(QStringConverter::Encoding encoding:{
                                        QStringConverter::Encoding::System,
                                        QStringConverter::Encoding::Latin1,
                                        QStringConverter::Encoding::Utf8,
                                        QStringConverter::Encoding::Utf16LE,
                                        QStringConverter::Encoding::Utf16BE,
                                        QStringConverter::Encoding::Utf32LE,
                                        QStringConverter::Encoding::Utf32BE,
                                    })
                                {
                                    auto&decoder=std::get<0>(encodings_states.at(encoding));
                                    QFontMetrics fontMetrics((*std::get<2>(encodings_states.at(encoding)))->font());
                                    co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                    QString result=decoder.decode(mimeData_copy_data);
                                    if(!decoder.hasError())
                                    {
                                        std::u32string result_remove_not_supported_characters=result.toStdU32String();
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        for(auto chunk:result_remove_not_supported_characters| std::views::chunk(1024))
                                        {
                                            std::ranges::replace_if(chunk,[&](char32_t c){return c>QChar::SpecialCharacter::LastValidCodePoint||!fontMetrics.inFontUcs4(c);},QChar::SpecialCharacter::ReplacementCharacter);
                                            co_await coroutine_post_to_same_thread_delay_t{100};
                                        }
                                        co_await coroutine_dispatch_to_thread_t{convert_string_thread()};
                                        result=QString::fromStdU32String(result_remove_not_supported_characters);

                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto [qplabel,qptext_edit]=std::make_tuple(*std::get<1>(encodings_states.at(encoding)),*std::get<2>(encodings_states.at(encoding)));!qplabel.isNull()&&!qptext_edit.isNull())
                                        {
                                            qptext_edit->setPlainText(result);
                                            qplabel->setEnabled(true);
                                            qptext_edit->setEnabled(true);
                                        }
                                    }
                                    else
                                    {
                                        co_await coroutine_dispatch_to_main_thread_t{};
                                        if(auto qptext_edit=*std::get<2>(encodings_states.at(encoding));!qptext_edit.isNull())
                                        {
                                            qptext_edit->setPlainText("decode error");
                                        }
                                    }
                                }
                            }(mimeData_copy->data(format),std::move(encodings_states),std::move(to_base64_states),std::move(from_base64_states),std::move(from_base64_strict_states),std::move(to_base64url_states),std::move(from_base64url_states),std::move(from_base64url_strict_states),std::move(to_percent_states),std::move(from_percent_states));
                            QPushButton*format_button=new QPushButton(format);
                            format_button->setCheckable(true);
                            formats_button_group->addButton(format_button,button_id);
                            ++button_id;
                            formats_flow_layout->addWidget(format_button);
                            formats_stacked_layout->addWidget(widget);
                        }
                        QObject::connect(copy_button,&QPushButton::clicked,copy_button,[mimeData_copy]()mutable{QGuiApplication::clipboard()->setMimeData(copyMimeData(mimeData_copy.get()));});
                        QObject::connect(formats_button_group,&QButtonGroup::idToggled,formats_stacked_layout,[formats_stacked_layout](int id, bool checked){if(checked){formats_stacked_layout->setCurrentIndex(id);}});
                        formats_button_group->button(0)->setChecked(true);
                    }
                }
            } });

    QWidget *clipboard_history_widget_outer = new QWidget();
    QVBoxLayout *clipboard_history_vlayout_outer = new QVBoxLayout(clipboard_history_widget_outer);
    clipboard_history_vlayout_outer->setContentsMargins(QMargins());
    clipboard_history_vlayout_outer->addWidget(clipboard_history_widget_inner);
    clipboard_history_vlayout_outer->addStretch(1);
    ScrollAreaVertical *mime_viewer_scroll_area = new ScrollAreaVertical(clipboard_history_widget_outer);

    return mime_viewer_scroll_area;
}
