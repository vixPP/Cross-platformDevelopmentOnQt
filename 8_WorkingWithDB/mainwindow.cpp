#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Исходное состояние виджетов
    ui->setupUi(this);

    ui->lb_statusConnect->setStyleSheet("color:red");
    ui->pb_request->setEnabled(false);

    /*
     * Выделим память под необходимые объекты. Все они наследники
     * QObject, поэтому воспользуемся иерархией.
    */

    dataDb = new DbData(this);
    dataBase = new DataBase(this);
    msg = new QMessageBox(this);

    //Установим размер вектора данных для подключения к БД
    dataForConnect.resize(NUM_DATA_FOR_CONNECT_TO_DB);

    /*
     * Добавим БД используя стандартный драйвер PSQL и зададим имя.
    */
    dataBase->AddDataBase(POSTGRE_DRIVER, DB_NAME);

    /*
     * Устанавливаем данные для подключениея к БД.
     * Поскольку метод небольшой используем лямбда-функцию.
     */
    connect(dataDb, &DbData::sig_sendData, this, [&](QVector<QString> receivData){
        dataForConnect = receivData;
    });

    /*
     * Соединяем сигнал, который передает ответ от БД с методом, который отображает ответ в ПИ
     */
     connect(dataBase, &DataBase::sig_SendDataFromDB, this, &MainWindow::ScreenDataFromDB);

    /*
     *  Сигнал для подключения к БД
     */
    connect(dataBase, &DataBase::sig_SendStatusConnection, this, &MainWindow::ReceiveStatusConnectionToDB);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * @brief Слот отображает форму для ввода данных подключения к БД
 */
void MainWindow::on_act_addData_triggered()
{
    //Отобразим диалоговое окно. Какой метод нужно использовать?
    dataDb->show();
}

/*!
 * @brief Слот выполняет подключение к БД. И отображает ошибки.
 */

void MainWindow::on_act_connect_triggered()
{
    /*
     * Обработчик кнопки у нас должен подключаться и отключаться от БД.
     * Можно привязаться к надписи лейбла статуса. Если он равен
     * "Отключено" мы осуществляем подключение, если "Подключено" то
     * отключаемся
    */

    if(ui->lb_statusConnect->text() == "Отключено"){

       ui->lb_statusConnect->setText("Подключение");
       ui->lb_statusConnect->setStyleSheet("color : black");


       auto conn = [&]{dataBase->ConnectToDataBase(dataForConnect);};
       QtConcurrent::run(conn);

    }
    else
    {
        dataBase->DisconnectFromDataBase(DB_NAME);
        ui->lb_statusConnect->setText("Отключено");
        ui->act_connect->setText("Подключиться");
        ui->lb_statusConnect->setStyleSheet("color:red");
        ui->pb_request->setEnabled(false);
    }

}

/*!
 * \brief Обработчик кнопки "Получить"
 */
void MainWindow::on_pb_request_clicked()
{


    uint8_t selectedGenre = (ui->cb_category->currentIndex() + 1);

    if (selectedGenre == requestAllFilms)
    {

//        queryModel = new QSqlQueryModel(this);
//        QString queryStr;
//         queryStr = "SELECT title, description FROM film JOIN film_category ON film.film_id = film_category.film_id JOIN category ON film_category.category_id = category.category_id";
//        queryModel->setQuery(queryStr, *dataBase->getDatabase());
//        queryModel->setHeaderData(0, Qt::Horizontal, tr("Movie Title"));
//        queryModel->setHeaderData(1, Qt::Horizontal, tr("Movie Description"));
//        ui->tableView->setModel(queryModel);

        tableModel = new QSqlTableModel(this, *dataBase->getDatabase());
        tableModel->setTable("film");
        tableModel->select();
        tableModel->setHeaderData(1, Qt::Horizontal, tr("Movie Title"));
        tableModel->setHeaderData(2, Qt::Horizontal, tr("Movie Description"));
        ui->tableView->setModel(tableModel);
        //Это конечно что то из темной Эры технологий но работает.
        for (int col = 0; col < tableModel->columnCount(); ++col)
        {
            if (col != 1 && col != 2)
            {
                ui->tableView->setColumnHidden(col, true);
            }
        }
    }
    else
    {
        queryModel = new QSqlQueryModel(this);
        QString queryStr;

        if (selectedGenre == requestComedy)
        {
            queryStr = "SELECT title, description FROM film JOIN film_category ON film.film_id = film_category.film_id JOIN category ON film_category.category_id = category.category_id WHERE category.name = 'Comedy'";
        }
        else if (selectedGenre == requestHorrors)
        {
            queryStr = "SELECT title, description FROM film JOIN film_category ON film.film_id = film_category.film_id JOIN category ON film_category.category_id = category.category_id WHERE category.name = 'Horror'";
        }

        queryModel->setQuery(queryStr, *dataBase->getDatabase());
        queryModel->setHeaderData(0, Qt::Horizontal, tr("Movie Title"));
        queryModel->setHeaderData(1, Qt::Horizontal, tr("Movie Description"));
        ui->tableView->setModel(queryModel);
    }



}

void MainWindow::on_pb_clear_clicked()
{
    ui->tableView->setModel(nullptr);
}
/*!
 * \brief Слот отображает значение в QTableWidget
 * \param widget
 * \param typeRequest
 */
void MainWindow::ScreenDataFromDB(const QTableWidget *widget, int typeRequest)
{

    ///Тут должен быть код ДЗ


}
/*!
 * \brief Метод изменяет стотояние формы в зависимости от статуса подключения к БД
 * \param status
 */
void MainWindow::ReceiveStatusConnectionToDB(bool status)
{
    if(status){
        ui->act_connect->setText("Отключиться");
        ui->lb_statusConnect->setText("Подключено к БД");
        ui->lb_statusConnect->setStyleSheet("color:green");
        ui->pb_request->setEnabled(true);
    }
    else{
        dataBase->DisconnectFromDataBase(DB_NAME);
        msg->setIcon(QMessageBox::Critical);
        msg->setText(dataBase->GetLastError().text());
        ui->lb_statusConnect->setText("Отключено");
        ui->lb_statusConnect->setStyleSheet("color:red");
        msg->exec();
    }

}



