#ifndef __PLUGINSINTERFACES_H__
#define __PLUGINSINTERFACES_H__

class TextEditInterface
{
public:
    virtual ~TextEditInterface() {}
	virtual QString menuName() const = 0;
    virtual QString text(QString text, QString selectedText) const = 0;
};

Q_DECLARE_INTERFACE(TextEditInterface,
                    "qdevelop.TextEditInterface/1.0")

#endif // __PLUGINSINTERFACES_H__
