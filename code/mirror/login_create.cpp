#include "login_create.h"
#include <QMessageBox>
#include <QFile>
#include <QMovie>
#include "RoleDefine.h"
#include "def_System_para.h"
#include "cryptography.h"
#include "MirrorVersion.h"

login_create::login_create(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.lbl_role_photo->setAttribute(Qt::WA_TranslucentBackground, true);

	m_vocation = Voc_Warrior;
	m_gender = 1;
	on_btn_vocation_1_clicked();
	bCreate = false;
}

login_create::~login_create()
{

}

void login_create::changePhoto()
{
	qint32 headNo = ((m_vocation - 1) * 2 + m_gender) * 10;
	QString path = (":/mirror/Resources/role/") + QString::number(headNo) + ".gif";

	QPoint pos;
	switch (headNo)
	{
	case 10: pos = QPoint(100, 40); break;
	case 20: pos = QPoint(50, 60); break;
	case 30: pos = QPoint(90, 40); break;
	case 40: pos = QPoint(60, 50); break;
	case 50: pos = QPoint(80, 40); break;
	case 60: pos = QPoint(70, 50); break;
	default: pos = QPoint(100, 40); break;
	}
	ui.lbl_role_photo->move(pos);

	QMovie *movie = new QMovie(path);
	ui.lbl_role_photo->setMovie(movie);
	movie->setSpeed(50);
	movie->start();
}
void login_create::on_btn_vocation_1_clicked()
{
	m_vocation = Voc_Warrior;
	changePhoto();
}
void login_create::on_btn_vocation_2_clicked()
{
	m_vocation = Voc_Magic;
	changePhoto();
}
void login_create::on_btn_vocation_3_clicked()
{
	m_vocation = Voc_Taoist;
	changePhoto();
}
void login_create::on_btn_gender_m_clicked()
{
	m_gender = 1;
	changePhoto();
}
void login_create::on_btn_gender_f_clicked()
{
	m_gender = 2;
	changePhoto();
}
void login_create::on_btn_ok_clicked()
{
	QString strTmp = ui.edit_role_name->text();
	if (strTmp.isEmpty())
	{
		QString title = QStringLiteral("提示");
		QString message = QStringLiteral("用户名不可为空。");
		QMessageBox msgBox(QMessageBox::Question, title, message);
		msgBox.addButton(QStringLiteral(" 是 "), QMessageBox::AcceptRole);
		msgBox.exec();
	}
	else
	{
		bCreate = CreateRole(strTmp);
		if (bCreate)
		{
			done(QDialog::Accepted);
		}
	}
}
void login_create::on_btn_quit_clicked()
{
	done(Rejected);
}

bool login_create::CreateRole(const QString &name)
{
	quint32 level;
	quint64 coin, gold, reputation, exp, lvExp;

	reputation = exp = 0;
	level = 1;
	coin = 50000;
	gold = 1000;
#ifdef _DEBUG
	exp = 900000000;
	coin = 10000000;
#endif

	QByteArray save_plain, save_cryptograph;
	char rolename[128] = {'\0'};
	sprintf_s(rolename, 128, "%s", name.toStdString().c_str());

	QDataStream out(&save_plain, QIODevice::WriteOnly);
	out << version_major << version_minor << version_build << SaveFileVer;
	//基本信息
	out.writeRawData(rolename, 128);
	out << m_vocation << m_gender;
	out << coin << gold << reputation << exp << level;

	//身上装备。
	Info_Equip onWearEquip[MaxEquipCountForRole] = { 0 };
	out.writeRawData((char *)&onWearEquip, sizeof(onWearEquip));

	//道具背包、道具仓库、装备背包、装备仓库皆为空。
	quint32 bag_item_size = 3;
	out << bag_item_size << 220001 << 1 << 220003 << 1 << 220004 << 1;

	quint32 store_item_size, bag_equip_size, store_equip_size;
	store_item_size = store_equip_size = 0;
	out << store_item_size;
	
	//背包内放置基本装备。
	Info_Equip equip = { 0 };
	QVector<itemID> VecEquip = { 301001, 302001, 303001 };
	out << VecEquip.size();
	for (quint32 i = 0; i < VecEquip.size(); i++)
	{
		equip.ID = VecEquip[i];
		out.writeRawData((char *)&equip, sizeof(Info_Equip));
	}

	out << store_equip_size;

	//已学技能列表
	quint32 skill_study_count = 0;
	out << skill_study_count;

	char *pchar = save_plain.data();

	if (!cryptography::Encrypt(save_cryptograph, save_plain))
	{
		return false;
	}

	QFile file(SaveFileName);
	if (!file.open(QIODevice::WriteOnly))
	{
		return false;
	}
	QDataStream dfs(&file);
	dfs.writeRawData(save_cryptograph.data(), save_cryptograph.size());
	dfs.writeRawData(save_plain.data(), save_plain.size());
	file.close();
	return true;
}